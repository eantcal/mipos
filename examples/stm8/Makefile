SDCC ?= sdcc
STM8SIM ?= ucsim_stm8

TARGET ?= stm8s_example
BUILD_DIR ?= build

MIPOSDIR := ../../mipos
MIPOSARCH := ../../mipos/arch

MCUFAM ?= stm8
MCUMODEL ?= STM8S208

SRCS = \
   main.c \
   $(MIPOSDIR)/mipos_kernel.c \
   $(MIPOSDIR)/mipos_task.c \
   $(MIPOSARCH)/$(MCUFAM)/mipos_bsp_$(MCUFAM)s.c \
   $(MIPOSARCH)/$(MCUFAM)/stm8s_lib/src/stm8s_tim4.c \
   $(MIPOSARCH)/$(MCUFAM)/stm8s_lib/src/stm8s_clk.c \
   $(MIPOSARCH)/$(MCUFAM)/stm8s_lib/src/stm8s_rst.c \
   $(MIPOSARCH)/$(MCUFAM)/stm8s_lib/src/stm8s_iwdg.c

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.rel,$(notdir $(SRCS)))

ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG
endif

INCLUDES += -I$(MIPOSARCH)/$(MCUFAM)/stm8s_lib/inc -I$(MIPOSDIR) -I$(MIPOSARCH)/$(MCUFAM)
CFLAGS += -m$(MCUFAM) -c --std-c99 --opt-code-size $(INCLUDES) -DMIPOS_TARGET_STM8=1 -D$(MCUMODEL)
LDFLAGS += -m$(MCUFAM) --out-fmt-ihx -lstm8

.PHONY: all clean flash sim

all: $(BUILD_DIR)/$(TARGET).ihx

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR) $(OBJS)
	$(SDCC) -o $(BUILD_DIR)/$(TARGET).ihx $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).ihx
	stm8flash -cstlink -pstm8l150 -w $(BUILD_DIR)/$(TARGET).ihx

sim: $(BUILD_DIR)/$(TARGET).ihx
	$(STM8SIM) -p "sstm8> " $(BUILD_DIR)/$(TARGET).ihx

$(BUILD_DIR)/main.rel: main.c | $(BUILD_DIR)
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/mipos_kernel.rel: $(MIPOSDIR)/mipos_kernel.c | $(BUILD_DIR)
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/mipos_task.rel: $(MIPOSDIR)/mipos_task.c | $(BUILD_DIR)
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/mipos_bsp_$(MCUFAM)s.rel: $(MIPOSARCH)/$(MCUFAM)/mipos_bsp_$(MCUFAM)s.c | $(BUILD_DIR)
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stm8s_%.rel: $(MIPOSARCH)/$(MCUFAM)/stm8s_lib/src/stm8s_%.c | $(BUILD_DIR)
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@
