#include "sys_common.h"
// Measurement Method used - please specify !!! 
#define PMU_Cycle 
//#define RTI_Cycle 
//#define GIO_Toggle
#ifdef PMU_Cycle 
#include "sys_pmu.h" 
#endif //PMU_Cycle
#ifdef RTI_Cycle 
#include "lld_TIM.h" 
/*
 RTI Clock Source Register (RCLKSRC), offset = 0x
 If the RTIx clock source is chosen to be anything other than the default VCLK,
 then the RTI clock needs to be at least three times slower than the VCLK.
 bit9..8 RTI1DIV[1:0]
 bit3..0 RTI1SRC[3:0], clock source x or VCLK (1000b..1111b, default = 1001b)
 */
#define f_RTICLK (float) 90.0 // f in [MHz]; RTICLK = VCLK = HCLK / 2 (depends on device setup) 
#define RTI_FRC0 0xfffffc10 // Counter0 
#endif //RTI_Cycle
#ifdef GIO_Toggle 
#include "lld_DIO.h" 
#endif //GIO_Toggle
#define f_HCLK (float) 180.0 // f in [MHz]; HCLK (depends on device setup)

void code_to_be_measured(void)
{

}
volatile unsigned int loop_count_prep, loop_count_prep_max = 1000;
volatile unsigned int loop_count, loop_count_max = 1000;

#ifdef PMU_Cycle 
volatile unsigned long cycles_PMU_start, cycles_PMU_end, cycles_PMU_measure,
		cycles_PMU_comp, cycles_PMU_code;
volatile float time_PMU_code;
#endif //PMU_Cycle
#ifdef RTI_Cycle 
volatile unsigned long i;
volatile unsigned long cycles_RTI_start, cycles_RTI_end, cycles_RTI_measure, cycles_RTI_comp, cycles_RTI_code, cycles_RTI;
volatile float time_RTI_code;
#endif //RTI_Cycle
#ifdef GIO_Toggle 
void OCDIO_Notification(IO_ChannelType channel, OCDIO_ValueType notifType) {
}
#endif //GIO_Toggle

void pmu_main() {
	// -- Measurement Initialization --
#ifdef PMU_Cycle
	_pmuInit_();
	_pmuEnableCountersGlobal_();
	_pmuSetCountEvent_(pmuCOUNTER0, PMU_CYCLE_COUNT); // PMU_INST_ARCH_EXECUTED
#endif //PMU_Cycle

#ifdef RTI_Cycle
	OCTIM_InitSync(OCTIM_0);
#endif //RTI_Cycle
#ifdef GIO_Toggle
	OCDIO_InitSync(OCDIO_0);
#endif //GIO_Toggle

	// -- Measurement Preparation --
	for (loop_count_prep = 0; loop_count_prep < loop_count_prep_max;
			++loop_count_prep) {
		// run benchmark code - function call or code sequence 
		code_to_be_measured();
		// or run preparation code
	}

	// -- Measurement Execution --
#ifdef PMU_Cycle
	_pmuResetCounters_();
	_pmuStartCounters_(pmuCOUNTER0);
	cycles_PMU_start = _pmuGetEventCount_(pmuCOUNTER0);
#endif //PMU_Cycle
#ifdef RTI_Cycle
	OCTIM_StartSync(0);
	cycles_RTI_start = OCTIM_GetSync(0);
	// cycles_RTI_start = (unsigned long) *((volatile unsigned long*) ((unsigned long) RTI_FRC0)); 
#endif //RTI_Cycle

#ifdef GIO_Toggle
	OCDIO_SetSync(0, 1); // OCDIO0->DSET = 0x01U; 
#endif //GIO_Toggle

	for (loop_count = 0; loop_count < loop_count_max; ++loop_count) // in case multiple loops are needed
			{
		// run benchmark code - function call or code sequence 
		code_to_be_measured();
	}

#ifdef GIO_Toggle
	OCDIO_SetSync(0, 0);
	// OCDIO0->DCLR = 0x01U; 
#endif //GIO_Toggle

#ifdef RTI_Cycle
	OCTIM_StopSync(0);
	cycles_RTI_end = OCTIM_GetSync(0);
	// cycles_RTI_end = (unsigned long) *((volatile unsigned long*) ((unsigned long) RTI_FRC0)); 
	cycles_RTI_measure = cycles_RTI_end - cycles_RTI_start;
#endif //RTI_Cycle

#ifdef PMU_Cycle
	_pmuStopCounters_(pmuCOUNTER0);
	cycles_PMU_end = _pmuGetEventCount_(pmuCOUNTER0);
	cycles_PMU_measure = cycles_PMU_end - cycles_PMU_start;
#endif //PMU_Cycle

	// -- Measurement Time Compensation --
#ifdef PMU_Cycle
	_pmuResetCounters_();
	_pmuStartCounters_(pmuCOUNTER0);
	cycles_PMU_start = _pmuGetEventCount_(pmuCOUNTER0);
#endif //PMU_Cycle
#ifdef RTI_Cycle
	OCTIM_StartSync(0);
	cycles_RTI_start = OCTIM_GetSync(0);
	// cycles_RTI_start = (unsigned long) *((volatile unsigned long*) ((unsigned long) RTI_FRC0)); 
#endif //RTI_Cycle

#ifdef GIO_Toggle
	OCDIO_SetSync(0, 1);
	// OCDIO0->DSET = 0x01U;

	OCDIO0->DSET = 0x01U;

	OCDIO_SetSync(0, 0);//OCDIO0->DCLR = 0x01U;
#endif //GIO_Toggle
#ifdef RTI_Cycle
	OCTIM_StopSync(0);
	cycles_RTI_end = OCTIM_GetSync(0);
	// cycles_RTI_end = (unsigned long) *((volatile unsigned long*) ((unsigned long) RTI_FRC0)); cycles_RTI_comp = cycles_RTI_end - cycles_RTI_start;
#endif //RTI_Cycle
#ifdef PMU_Cycle
	_pmuStopCounters_(pmuCOUNTER0);
	cycles_PMU_end = _pmuGetEventCount_(pmuCOUNTER0);
	cycles_PMU_comp = cycles_PMU_end - cycles_PMU_start;
#endif //PMU_Cycle

	// -- Code Cycles / Run Time Calculation --
#ifdef PMU_Cycle
	cycles_PMU_code = cycles_PMU_measure - cycles_PMU_comp;
	time_PMU_code = cycles_PMU_code / (f_HCLK); // time_code [us], f_HCLK [MHz]
	//time_PMU_code= cycles_PMU_code / (f_HCLK * loop_Count_max); //
#endif //PMU_Cycle
#ifdef RTI_Cycle
cycles_RTI = (cycles_RTI_measure - cycles_RTI_comp);
/*
 RTI Compare UpCounter 0 Register (RTICPUC0), offset = 0x18:
 CPUC0 = 0 ==>RTICLK / 2^32
 CPUC0 /= 0 ==>RTICLK / (n+1)
 CPUC0 = 1 ==>RTICLK / 2
 */
/*
 RTICLK = VCLK = HCLK / 2
 */
cycles_RTI_code = cycles_RTI * 4;
/* factor 2*2 to compensate counting of every 2nd VCLK
 clock pulse */
time_RTI_code = cycles_RTI_code / (f_HCLK); // time_code [us], f_HCLK [MHz]
#endif //RTI_Cycle
#ifdef GIO_Toggle
// to be measured with oscilloscope
#endif //GIO_Toggle

	while (1);
}

