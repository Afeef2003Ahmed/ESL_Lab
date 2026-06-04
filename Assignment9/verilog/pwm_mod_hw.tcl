##-----------------------------------------------------------------------------
##
## This file is seperated in the following sections:
## - IP details
## - Required files
## - IP parameters
## - Interface specifications
## - Validation/ elaboration functions
##
##-----------------------------------------------------------------------------

## 
## IP details
##  
set_module_property DESCRIPTION "PWM IP (Avalon-MM slave port)"
set_module_property NAME pwm_mod_wrapper
set_module_property VERSION 1.0
set_module_property GROUP Templates
set_module_property AUTHOR student
set_module_property DISPLAY_NAME pwm_ip
set_module_property TOP_LEVEL_HDL_FILE pwm_mod_wrapper.v
set_module_property TOP_LEVEL_HDL_MODULE pwm_mod_wrapper
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE false
set_module_property SIMULATION_MODEL_IN_VERILOG false
set_module_property SIMULATION_MODEL_IN_VHDL false
set_module_property SIMULATION_MODEL_HAS_TULIPS false
set_module_property SIMULATION_MODEL_IS_OBFUSCATED false

## 
## Link to the verification methods
## Defined at the bottom of this file
##  
set_module_property ELABORATION_CALLBACK elaborate_me
set_module_property VALIDATION_CALLBACK validate_me

## 
## Files
## - List all files required by the IP
##  
add_file pwm_mod_wrapper.v {SYNTHESIS SIMULATION}
add_file pwm_mod.v         {SYNTHESIS SIMULATION}

## 
## IP parameters
## - Parameters defined in the Verilog can be modified from
##   a wizard in Platform Designer. This section defines how
##   these parameters are presented to the user.
## - The actual link between the parameters and the generics
##   is made in the "elaborate_me" function.
##
add_parameter DATA_WIDTH int 32 "Data width for avalon interface"
set_parameter_property DATA_WIDTH DISPLAY_NAME "Word Size"
set_parameter_property DATA_WIDTH GROUP "Register File Properties"
set_parameter_property DATA_WIDTH AFFECTS_PORT_WIDTHS true
set_parameter_property DATA_WIDTH ALLOWED_RANGES {8 16 32}

add_parameter PWM_FREQ int 20000 "PWM output frequency in Hz (must be >= 20000)"
set_parameter_property PWM_FREQ DISPLAY_NAME "PWM Frequency (Hz)"
set_parameter_property PWM_FREQ GROUP "PWM Properties"
set_parameter_property PWM_FREQ AFFECTS_PORT_WIDTHS false
set_parameter_property PWM_FREQ AFFECTS_ELABORATION true
set_parameter_property PWM_FREQ ALLOWED_RANGES 20000:1000000

add_parameter CLK_FREQ int 50000000 "Input clock frequency in Hz"
set_parameter_property CLK_FREQ DISPLAY_NAME "Clock Frequency (Hz)"
set_parameter_property CLK_FREQ GROUP "PWM Properties"
set_parameter_property CLK_FREQ AFFECTS_PORT_WIDTHS false
set_parameter_property CLK_FREQ AFFECTS_ELABORATION true
set_parameter_property CLK_FREQ ALLOWED_RANGES 1000000:200000000

## 
## Interface
## - Add clock and reset signals
##  
add_interface clock_reset clock end
set_interface_property clock_reset ptfSchematicName ""

add_interface_port clock_reset clk   clk   Input 1
add_interface_port clock_reset reset reset Input 1

##
## - Add the avalon interface
##   The properties descriptions may be found in the
##   avalon interface specifications.
##
add_interface s0 avalon end
set_interface_property s0 holdTime 0
set_interface_property s0 linewrapBursts false
set_interface_property s0 minimumUninterruptedRunLength 1
set_interface_property s0 bridgesToMaster ""
set_interface_property s0 isMemoryDevice false
set_interface_property s0 burstOnBurstBoundariesOnly false
set_interface_property s0 addressSpan 512
set_interface_property s0 timingUnits Cycles
set_interface_property s0 setupTime 0
set_interface_property s0 writeWaitTime 0
set_interface_property s0 isNonVolatileStorage false
set_interface_property s0 addressAlignment DYNAMIC
set_interface_property s0 maximumPendingReadTransactions 0
set_interface_property s0 readWaitTime 0
set_interface_property s0 readLatency 1
set_interface_property s0 printableDevice false
set_interface_property s0 ASSOCIATED_CLOCK clock_reset

add_interface_port s0 slave_address   address   Input  3
add_interface_port s0 slave_read      read      Input  1
add_interface_port s0 slave_write     write     Input  1
add_interface_port s0 slave_readdata  readdata  Output -1
add_interface_port s0 slave_writedata writedata Input  -1


##
add_interface pwm_interface conduit end
add_interface_port pwm_interface INA     export Output 1
add_interface_port pwm_interface INB     export Output 1
add_interface_port pwm_interface pwm_out export Output 1

##
## - Validation/ elaboration functions
##
proc validate_me {} {
    set pwm_freq [get_parameter_value PWM_FREQ]
    set clk_freq [get_parameter_value CLK_FREQ]

    if { $pwm_freq < 20000 } {
        send_message error "PWM frequency must be at least 20000 Hz to stay outside human hearing range."
    }

    if { $clk_freq < $pwm_freq } {
        send_message error "Clock frequency must be greater than PWM frequency."
    }
}

proc elaborate_me {} {
    ## Retrieve the parameters from the wizard
    set the_data_width [get_parameter_value DATA_WIDTH]
    set the_pwm_freq   [get_parameter_value PWM_FREQ]
    set the_clk_freq   [get_parameter_value CLK_FREQ]

    ## Set data width for the avalon interface
    set_port_property slave_readdata  WIDTH $the_data_width
    set_port_property slave_writedata WIDTH $the_data_width

    ## DO NOT REMOVE:
    ## Add slave_byteenable only if data width is greater than 8 bits
    if { $the_data_width != 8 } {
        add_interface_port s0 slave_byteenable byteenable Input [expr {$the_data_width / 8}]
    }
}
