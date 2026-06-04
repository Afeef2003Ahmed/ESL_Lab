`include "quadrature_enc.v"

module quadrature_enc_tb();

reg clk;
reg enc_a;
reg enc_b;
wire direction;
wire [31:0] counter;

integer i;

quadrature_enc dut (.clk(clk),.enc_a(enc_a),.enc_b(enc_b),
                        .direction(direction),
                        .counter(counter));

always #5 clk = ~clk;

initial begin

    $dumpfile("signals.vcd");
    $dumpvars(0, quadrature_enc_tb);

    clk = 0;
    enc_a = 0;
    enc_b = 0;



    for (i = 0; i < 10; i = i + 1) begin

        #10 enc_a = 1;
        #10 enc_b = 1;
        #10 enc_a = 0;
        #10 enc_b = 0;

    end

    for (i = 10; i >0; i = i - 1) begin

        #10 enc_b = 1;
        #10 enc_a = 1;
        #10 enc_b = 0;
        #10 enc_a = 0;


    end

    #50 $finish;

    
end
endmodule
