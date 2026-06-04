module quadrature_enc (
    input clk,
    input enc_a,
    input enc_b,
    output reg[31:0] counter = 0,
    output reg direction = 0);

reg enc_a_prev = 0;

always @(posedge clk) begin

    if (enc_a == 1 && enc_a_prev == 0) begin

        if (enc_b == 0) begin

            direction <= 1;
            counter <= counter + 1;

        end

        else begin

            direction <= 0;
            counter <= counter - 1;

        end


    end

    enc_a_prev = enc_a;

end


endmodule