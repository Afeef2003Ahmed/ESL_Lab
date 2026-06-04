module multiLedBlink(
    input clk,
    output reg led1 = 0,
    output reg led2 = 0, 
    output reg led3 = 0
);

reg [31:0] count = 0;

always@(posedge clk) begin

    if (count >= 99999999) begin

        led1 <= ~led1;
        count <= 0;
        
    end
    else begin

        count <= count + 1;

    end

    if (count >= 49999999) begin

        led2 <= ~led2; // led2 blinks 2x fast

    end

    if (count >= 24999999) begin

        led3 <= ~led3; // led3 blinks 4x fast

    end
    
end

endmodule