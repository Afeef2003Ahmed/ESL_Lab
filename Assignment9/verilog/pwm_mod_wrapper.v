module pwm_mod_wrapper #(
    parameter PWM_FREQ   = 20_000,
    parameter CLK_FREQ   = 50_000_000,
    parameter DATA_WIDTH = 32
)(
    input  wire        clk,
    input  wire        reset,

    input  wire [2:0]  slave_address,
    input  wire        slave_read,
    output reg  [DATA_WIDTH-1:0] slave_readdata,
    input  wire        slave_write,
    input  wire [DATA_WIDTH-1:0] slave_writedata,
    input  wire [(DATA_WIDTH/8)-1:0] slave_byteenable,

    output wire        INA,
    output wire        INB,
    output wire        pwm_out,
    output wire        led_out
);

reg [31:0] mem;

wire enable     = mem[31];
wire direction  = mem[30];
wire [6:0] duty_cycle = mem[6:0];

pwm_mod #(
    .PWM_FREQ(PWM_FREQ),
    .CLK_FREQ(CLK_FREQ)
) my_ip (
    .clk       (clk),
    .rst       (reset),
    .enable    (enable),
    .duty_cycle(duty_cycle),
    .direction (direction),
    .INA       (INA),
    .INB       (INB),
    .pwm_out   (pwm_out)
);

assign led_out = pwm_out;

always @(posedge clk or posedge reset) begin
    if (reset) begin
        mem            <= 32'b0;
        slave_readdata <= 32'b0;
    end else begin
        if (slave_read)  slave_readdata <= mem;
        if (slave_write) mem <= slave_writedata;
    end
end

endmodule