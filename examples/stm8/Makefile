SDCC=sdcc
SDLD=sdld

TARGET=stm8s_example

MIPOSDIR=../../mipos
MIPOSARCH=../../mipos/arch

MCUFAM=stm8
MCUMODEL=STM8S208
SRCS=$(MIPOSDIR)/mipos_kernel.c 

OBJS = \
   main.rel \
   $(MIPOSDIR)/mipos_kernel.rel \
   $(MIPOSDIR)/mipos_task.rel \
   $(MIPOSARCH)/$(MCUFAM)/mipos_bsp_$(MCUFAM)s.rel \
   $(MIPOSARCH)/$(MCUFAM)/stm8s-sdcc/src/stm8s_tim4.rel \
   $(MIPOSARCH)/$(MCUFAM)/stm8s-sdcc/src/stm8s_clk.rel \
   $(MIPOSARCH)/$(MCUFAM)/stm8s-sdcc/src/stm8s_rst.rel \
   $(MIPOSARCH)/$(MCUFAM)/stm8s-sdcc/src/stm8s_iwdg.rel
   

ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG
endif

INCLUDES += -I$(MIPOSARCH)/$(MCUFAM)/stm8s-sdcc/inc -I$(MIPOSDIR) -I$(MIPOSARCH)/$(MCUFAM)
CFLAGS += -m$(MCUFAM) -c $(INCLUDES) -DMIPOS_TARGET_STM8 -D$(MCUMODEL)
LDFLAGS += -m$(MCUFAM) --out-fmt-ihx -lstm8

.PHONY: all clean flash

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(SDCC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET).ihx

flash: $(TARGET).ihx
	stm8flash -cstlink -pstm8l150 -w $(TARGET).ihx

%.rel: %.c
	@echo "CC: $<"
	$(SDCC) $(CFLAGS) $< -o $@;
