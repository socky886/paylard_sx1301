################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 --include_path="D:/reter sx1301/paylard_tms570ls1224_sx1301" --include_path="C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/include" --include_path="D:/reter sx1301/paylard_tms570ls1224_sx1301/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --enum_type=packed --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

