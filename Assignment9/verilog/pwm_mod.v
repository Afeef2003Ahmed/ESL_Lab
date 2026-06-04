module pwm_mod #(parameter PWM_FREQ = 20_000,
                 parameter CLK_FREQ = 50_000_000)(
    input  wire       clk,
    input  wire       rst,
    input  wire       enable,
    input  wire [6:0] duty_cycle,
    input  wire       direction,

    output reg        INA,
    output reg        INB,
    output reg        pwm_out
);

localparam integer PERIOD_SCALED = (CLK_FREQ / PWM_FREQ) / 100; 

reg [12:0] counter;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        counter <= 0;
        INA     <= 1'b0;
        INB     <= 1'b0;
        pwm_out <= 1'b0;
    end else begin
        if (!enable) begin
            counter <= 0;
            INA     <= 1'b0;
            INB     <= 1'b0;
            pwm_out <= 1'b0;
        end else begin
           
            INA <= (direction == 1'b0) ? 1'b1 : 1'b0;
            INB <= (direction == 1'b0) ? 1'b0 : 1'b1;

           
            if (counter >= (PERIOD_SCALED * 100) - 1)
                counter <= 0;
            else
                counter <= counter + 1;

            
            pwm_out <= (counter < duty_cycle * PERIOD_SCALED) ? 1'b1 : 1'b0;
        end
    end
end

endmodule