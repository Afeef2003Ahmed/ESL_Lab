module TopEntity (
    input clk,
    output reg led1 = 0
);
  reg [31:0] count = 0;
  always @(posedge clk) begin
    if (count == 5) begin  //Time is up // 99999999
      count <= 0;  //Reset count register
      led1   <= ~led1;  //Toggle led (in each second)
    end else begin
      count <= count + 1;  //Counts 100MHz clock
    end
  end
endmodule
