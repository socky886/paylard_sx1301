################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Hardware_Connections/%.obj: ../Hardware_Connections/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7R4 --code_state=32 --include_path="D:/reter sx1301/TC_TQ_TX" --include_path="D:/reter sx1301/TC_TQ_TX/UVMODULE" --include_path="D:/reter sx1301/TC_TQ_TX/Hardware_Connections" --include_path="D:/reter sx1301/TC_TQ_TX/include" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --enum_type=packed --abi=eabi --preproc_with_compile --preproc_dependency="Hardware_Connections/$(basename $(<F)).d_raw" --obj_directory="Hardware_Connections" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

