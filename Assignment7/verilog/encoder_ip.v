module encoder_ip (
    input clk,
    input rst,
    input enc_a,
    input enc_b,
    output reg[31:0] counter = 0,
    output reg direction = 0
);

reg enc_a_sync1, enc_a_sync2;
reg enc_b_sync1, enc_b_sync2;
reg [1:0] prev_state = 0;
reg [1:0] state = 0;

always @(posedge clk or posedge rst) begin
    if (rst) begin
        enc_a_sync1 <= 0;
        enc_a_sync2 <= 0;
        enc_b_sync1 <= 0;
        enc_b_sync2 <= 0;
        counter <= 32'b0;
        prev_state <= 2'b00;
        state <= 2'b00;
    end else begin
        
        enc_a_sync1 <= enc_a;
        enc_b_sync1 <= enc_b;
        
        enc_a_sync2 <= enc_a_sync1;
        enc_b_sync2 <= enc_b_sync1;
        
        
        state <= {enc_a_sync2, enc_b_sync2};
        prev_state <= state;
        
        case ({prev_state, state}) 
            4'b0010,4'b1011,4'b1101,4'b0100: begin 
                direction <= 1'b1; 
                counter <= counter + 32'b1;
            end
            4'b0001,4'b0111,4'b1110,4'b1000: begin 
                direction <= 1'b0;
                counter <= counter - 32'b1;
            end
        endcase
    end
end

endmodule