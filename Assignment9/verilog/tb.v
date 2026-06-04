`timescale 1ns / 1ps
// -----------------------------------------------------------------------------
// pwm_mod_tb.v  –  Testbench for pwm_mod
//
// Tests:
//   1. Reset behaviour        – all outputs low
//   2. Disabled               – all outputs low, counter held at 0
//   3. Forward  25% duty      – INA=1, INB=0, pwm_out high 25% of period
//   4. Forward  50% duty      – INA=1, INB=0, pwm_out high 50% of period
//   5. Forward 100% duty      – INA=1, INB=0, pwm_out always high
//   6. Reverse  50% duty      – INA=0, INB=1, pwm_out high 50% of period
//   7. 0% duty                – pwm_out always low, INA/INB still driven
//
// Simulate with:
//   iverilog -o pwm_mod_tb.vvp pwm_mod_tb.v pwm_mod.v
//   vvp pwm_mod_tb.vvp
//   gtkwave signals.vcd
// -----------------------------------------------------------------------------

`include "pwm_mod.v"

module pwm_mod_tb;

    // -------------------------------------------------------------------------
    // Use small parameters so simulation runs in microseconds, not seconds.
    // PERIOD_SCALED = CLK_FREQ / PWM_FREQ / 100 = 1000 / 10 / 100 = 1
    // One full PWM period = 100 counter ticks = 100 * 2 ns = 200 ns
    // -------------------------------------------------------------------------
    localparam CLK_FREQ = 1_000;   // 1 kHz – tiny, fast simulation
    localparam PWM_FREQ = 10;      // 10 Hz  – PERIOD = 100 ticks

    // DUT signals
    reg        clk       = 0;
    reg        rst       = 0;
    reg        enable    = 0;
    reg  [6:0] duty_cycle = 0;
    reg        direction = 0;
    wire       INA;
    wire       INB;
    wire       pwm_out;

    // Instantiate DUT
    pwm_mod #(
        .PWM_FREQ(PWM_FREQ),
        .CLK_FREQ(CLK_FREQ)
    ) dut (
        .clk       (clk),
        .rst       (rst),
        .enable    (enable),
        .duty_cycle(duty_cycle),
        .direction (direction),
        .INA       (INA),
        .INB       (INB),
        .pwm_out   (pwm_out)
    );

    // 2 ns clock period
    always #1 clk = ~clk;

    // -------------------------------------------------------------------------
    // Utility task: run for N full PWM periods and count pwm_out high cycles
    // -------------------------------------------------------------------------
    integer high_count;
    integer total_count;

    task run_and_measure;
        input integer periods;
        input integer period_ticks;
        begin
            high_count  = 0;
            total_count = periods * period_ticks;
            repeat (total_count) begin
                @(posedge clk);
                if (pwm_out) high_count = high_count + 1;
            end
        end
    endtask

    // -------------------------------------------------------------------------
    // Check helper: print PASS / FAIL
    // -------------------------------------------------------------------------
    task check;
        input [127:0] test_name;
        input         condition;
        begin
            if (condition)
                $display("PASS  %0s", test_name);
            else
                $display("FAIL  %0s", test_name);
        end
    endtask

    // -------------------------------------------------------------------------
    // Main stimulus
    // -------------------------------------------------------------------------
    localparam PERIOD_TICKS = CLK_FREQ / PWM_FREQ; // 100 ticks per period

    initial begin
        $dumpfile("signals.vcd");
        $dumpvars(0, pwm_mod_tb);

        // ------------------------------------------------------------------
        // Test 1: Reset
        // ------------------------------------------------------------------
        rst = 1;
        @(posedge clk); @(posedge clk);
        check("Reset: INA=0",     INA     == 1'b0);
        check("Reset: INB=0",     INB     == 1'b0);
        check("Reset: pwm_out=0", pwm_out == 1'b0);
        rst = 0;

        // ------------------------------------------------------------------
        // Test 2: Disabled (enable=0)
        // ------------------------------------------------------------------
        enable    = 0;
        duty_cycle = 50;
        direction = 0;
        repeat (PERIOD_TICKS * 2) @(posedge clk);
        check("Disabled: INA=0",     INA     == 1'b0);
        check("Disabled: INB=0",     INB     == 1'b0);
        check("Disabled: pwm_out=0", pwm_out == 1'b0);

        // ------------------------------------------------------------------
        // Test 3: Forward, 25% duty
        // ------------------------------------------------------------------
        enable    = 1;
        direction = 0;
        duty_cycle = 25;
        // Wait one period for counter to settle, then measure 4 periods
        repeat (PERIOD_TICKS) @(posedge clk);
        run_and_measure(4, PERIOD_TICKS);
        check("Fwd 25%: INA=1",   INA == 1'b1);
        check("Fwd 25%: INB=0",   INB == 1'b0);
        // Allow ±2 ticks tolerance due to off-by-one at wrap boundary
        check("Fwd 25%: duty",    high_count >= (total_count*25/100 - 2) &&
                                  high_count <= (total_count*25/100 + 2));
        $display("      measured %0d / %0d high ticks (~%0d%%)",
                 high_count, total_count, high_count*100/total_count);

        // ------------------------------------------------------------------
        // Test 4: Forward, 50% duty
        // ------------------------------------------------------------------
        duty_cycle = 50;
        repeat (PERIOD_TICKS) @(posedge clk);
        run_and_measure(4, PERIOD_TICKS);
        check("Fwd 50%: duty",    high_count >= (total_count*50/100 - 2) &&
                                  high_count <= (total_count*50/100 + 2));
        $display("      measured %0d / %0d high ticks (~%0d%%)",
                 high_count, total_count, high_count*100/total_count);

        // ------------------------------------------------------------------
        // Test 5: Forward, 100% duty
        // ------------------------------------------------------------------
        duty_cycle = 100;
        repeat (PERIOD_TICKS) @(posedge clk);
        run_and_measure(4, PERIOD_TICKS);
        check("Fwd 100%: pwm always high", high_count == total_count);
        $display("      measured %0d / %0d high ticks (~%0d%%)",
                 high_count, total_count, high_count*100/total_count);

        // ------------------------------------------------------------------
        // Test 6: Reverse, 50% duty
        // ------------------------------------------------------------------
        direction  = 1;
        duty_cycle = 50;
        repeat (PERIOD_TICKS) @(posedge clk);
        run_and_measure(4, PERIOD_TICKS);
        check("Rev 50%: INA=0",   INA == 1'b0);
        check("Rev 50%: INB=1",   INB == 1'b1);
        check("Rev 50%: duty",    high_count >= (total_count*50/100 - 2) &&
                                  high_count <= (total_count*50/100 + 2));
        $display("      measured %0d / %0d high ticks (~%0d%%)",
                 high_count, total_count, high_count*100/total_count);

        // ------------------------------------------------------------------
        // Test 7: 0% duty – pwm_out always low
        // ------------------------------------------------------------------
        direction  = 0;
        duty_cycle = 0;
        repeat (PERIOD_TICKS) @(posedge clk);
        run_and_measure(4, PERIOD_TICKS);
        check("Fwd 0%: INA=1",          INA == 1'b1);
        check("Fwd 0%: pwm always low",  high_count == 0);
        $display("      measured %0d / %0d high ticks", high_count, total_count);

        $display("\nSimulation complete.");
        $finish;
    end

endmodule