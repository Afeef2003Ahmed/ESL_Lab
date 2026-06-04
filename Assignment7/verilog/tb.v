`include "encoder_ip.v"

module tb();

reg clk;
reg rst;

reg enc_a;
reg enc_b;

wire [31:0] counter;
wire direction;
integer i;

encoder_ip dut (.clk(clk), .rst(rst), .enc_a(enc_a),.enc_b(enc_b),
                .counter(counter), .direction(direction));

always #5 clk = ~clk;

always@(counter) begin 
        $display("Counter %d : Direction %s",counter,direction ? "CW":"CCW");
    end

initial begin 

    $dumpfile("signals.vcd");
    $dumpvars(0,tb);

    clk = 0;
    enc_a = 0;
    enc_b = 0;

    for (i = 0; i<10; i = i+1) begin 
    #10 enc_a = 0; enc_b = 0;
    #10 enc_a = 1; enc_b = 0;
    #10 enc_a = 1; enc_b = 1;
    #10 enc_a = 0; enc_b = 1;

    end

    for (i = 10; i>0; i= i-1) begin 
    #10 enc_a = 0; enc_b = 0;
    #10 enc_a = 0; enc_b = 1;
    #10 enc_a = 1; enc_b = 1;
    #10 enc_a = 1; enc_b = 0;

    end

    #50 $finish;

end

endmodule
