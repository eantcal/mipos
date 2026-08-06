# Storage And FatFs

mipOS has two filesystem paths:

- the tiny mipOS filesystem (`mipos_tfs`) for compact RAM-backed examples;
- FatFs integration through `mipos_diskio` for FAT-formatted block devices.

This page focuses on the FatFs path because it is the one intended to scale from
host simulation to QEMU block devices and eventually physical SD/MMC storage.

## Layering

The FatFs stack in mipOS is deliberately split into four layers:

```text
Application commands or tasks
  |
FatFs API: f_mount, f_open, f_read, f_write, f_readdir
  |
FatFs diskio glue: fatfs/diskio.c
  |
mipOS disk registry: mipos/mipos_diskio.c
  |
One physical or simulated block driver
```

The key design point is that FatFs is not modified for every board. Board and
simulator differences are isolated behind `mipos_diskio` drivers.

## Disk Driver Registry

`mipos/mipos_diskio.c` keeps a small static registry of physical disk drivers:

```c
static mipos_disk_t _mipos_disk_drv[MIPOS_PH_DISK_DRVS] = { 0 };
static int _mipos_disk_drv_cnt = 0;
```

A driver is registered with `mipos_add_disk_driver`:

```c
if (_mipos_disk_drv_cnt < MIPOS_PH_DISK_DRVS) {
    ++_mipos_disk_drv_cnt;
} else {
    mipos_leave_cs();
    return -1;
}

memcpy(
  &_mipos_disk_drv[_mipos_disk_drv_cnt - 1], drv_ptr, sizeof(mipos_disk_t));
```

The registry dispatches each FatFs disk operation by physical drive number:

```c
int mipos_disk_read(mipos_pdrv_t pdrv,
                    char* buf,
                    mipos_sec_t sec,
                    mipos_sec_t count)
{
    mipos_disk_t const* drv_ptr = mipos_find_disk_driver(pdrv);

    return drv_ptr ? drv_ptr->_disk_read(pdrv, buf, sec, count)
                   : MIPOS_DISK_RES_NOTRDY;
}
```

This is the interface a future SDMMC, SPI SD-card, virtio block, or host-backed
QEMU disk driver must implement.

## QEMU Host Disk Driver

The current QEMU FatFs demo uses a host file opened through ARM semihosting. The
driver lives in `mipos/arch/qemu-arm/mipos_qemu_hostdisk.c` and exposes physical
drive `0`.

```c
static const mipos_disk_t g_hostdisk_driver = {
    0,
    hostdisk_init,
    hostdisk_status,
    hostdisk_read,
    hostdisk_write,
    hostdisk_ioctl
};
```

Initialization opens `qemu-fatfs.img`, checks that its size is a multiple of the
sector size, and records the sector count:

```c
g_handle =
  mipos_qemu_semihost_open(g_image_path, MIPOS_QEMU_SEMIHOST_OPEN_RBP);

image_size = mipos_qemu_semihost_flen(g_handle);
if (image_size <= 0 ||
    (image_size % MIPOS_QEMU_HOSTDISK_SECTOR_SIZE) != 0) {
    mipos_qemu_semihost_close(g_handle);
    g_handle = -1;
    g_status = MIPOS_DISK_STS_NOINIT;
    return g_status;
}

g_sector_count =
  (uint32_t)(image_size / MIPOS_QEMU_HOSTDISK_SECTOR_SIZE);
```

Read and write are sector based. The driver rejects out-of-range accesses before
calling semihosting:

```c
if (hostdisk_check_range(sector, count) != 0 ||
    hostdisk_seek_sector(sector) != 0) {
    return MIPOS_DISK_RES_INVPAR;
}

if (mipos_qemu_semihost_read(g_handle, buf, bytes) != (int)bytes) {
    return MIPOS_DISK_RES_ERROR;
}
```

## Semihosting Boundary

The QEMU host-disk driver uses ARM semihosting calls wrapped in
`mipos/arch/qemu-arm/mipos_qemu_semihosting.c`. The low-level operation is a
`bkpt 0xab` call with operation code in `r0` and argument pointer in `r1`:

```c
static int semihost_call(int operation, void* argument)
{
    register int r0 __asm("r0") = operation;
    register void* r1 __asm("r1") = argument;

    __asm volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");

    return r0;
}
```

This is only available when QEMU is launched with semihosting enabled. That is
why the run scripts add semihosting options for the `fatfs` firmware and start
QEMU from the build directory containing `qemu-fatfs.img`.

## QEMU FatFs Firmware

The QEMU FatFs example lives in `examples/qemu-arm/fatfs/main.c`. Its root task
registers the host-disk driver, initializes the console, mounts drive `0:`, and
then exposes filesystem commands.

```c
mipos_add_disk_driver(mipos_qemu_hostdisk_driver());

mipos_console_register_cmd_list(user_cmd_list);
mipos_console_init(&cinit);

mount_result = f_mount(&g_fat_fs, "0:", 1);
```

The command list is intentionally small:

```c
static mipos_console_cmd_t user_cmd_list[] = {
    { "ls", exec_cmd_ls, "ls [dir]          - list FAT directory" },
    { "cd", exec_cmd_cd, "cd [dir]          - change FAT directory" },
    { "pwd", exec_cmd_pwd, "pwd              - print FAT directory" },
    { "cat", exec_cmd_cat, "cat <file>       - print FAT file" },
    { "write", exec_cmd_write, "write <file> <s> - append text" },
    CMD_LIST_END
};
```

`cat` uses normal FatFs calls:

```c
res = f_open(&file, path, FA_READ);
...
res = f_read(&file, buffer, sizeof(buffer) - 1u, &bread);
```

`write` opens in append mode and writes one line:

```c
res = f_open(&file, path, FA_WRITE | FA_OPEN_APPEND);
...
res = f_write(&file, argv[2], strlen(argv[2]), &written);
```

## Running The QEMU Demo

Use the QEMU helper so the disk image and semihosting mode are configured
consistently:

```powershell
.\scripts\run-qemu.ps1 fatfs
```

```sh
./scripts/run-qemu.sh fatfs
```

The helper copies the default image from `examples/simu/fatfs/disk.img` to
`build-qemu-arm/qemu-fatfs.img` when the build image is missing. Use the reset
option when you want to discard writes and start from the original image again.

Inside QEMU:

```text
mipOS-fat>ls
mipOS-fat>cat <file>
mipOS-fat>write log.txt hello
mipOS-fat>cat log.txt
```

If the firmware prints `f_mount failed`, first check that QEMU was launched
through the helper or with semihosting enabled and that `qemu-fatfs.img` exists
in the current working directory.

## Porting To Physical SD/MMC

For a physical board, keep the same upper layers and replace only the block
driver below `mipos_diskio`:

1. initialize the SD/MMC or SPI block device in the BSP or board driver;
2. implement `_disk_status`, `_disk_init`, `_disk_read`, `_disk_write`, and
   `_disk_ioctl` for physical drive `0`;
3. register the driver with `mipos_add_disk_driver`;
4. mount `0:` with `f_mount`;
5. run simple `ls`, `cat`, and append/write commands before adding higher-level
   storage workflows.

The first physical driver should be conservative: single-block read/write first,
then multi-block transfers, then cache or DMA. Correct range checks and sector
size reporting matter more than throughput during bring-up.

## Related Pages

- [Architecture](Architecture)
- [QEMU ARM Targets](QEMU-ARM-Targets)
- [Porting Guide](Porting-Guide)
- [Build and Test](Build-and-Test)