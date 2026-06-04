`timescale 1 ps / 1 ps

module TopEntity (
    input  clk,
    input  SPI_CLK,
    input  SPI_PICO,
    input  SPI_CS,
    output SPI_POCI,
    output led1,   
    output led2,  
    output led3   
);
reg [2:0] sclk_r;
always @(posedge clk) sclk_r <= {sclk_r[1:0], SPI_CLK};
wire sclk_rise = (sclk_r[2:1] == 2'b01);
wire sclk_fall = (sclk_r[2:1] == 2'b10);

reg [2:0] cs_r;
always @(posedge clk) cs_r <= {cs_r[1:0], SPI_CS};
wire cs_active = ~cs_r[1];
wire cs_start  = (cs_r[2:1] == 2'b10);  // CS falling edge

reg [1:0] pico_r;
always @(posedge clk) pico_r <= {pico_r[0], SPI_PICO};
wire pico_data = pico_r[1];

reg [2:0] bit_cnt;
reg [7:0] rx_byte;
reg       byte_received;

always @(posedge clk) begin
    if (!cs_active)
        bit_cnt <= 3'd0;
    else if (sclk_rise) begin
        bit_cnt <= bit_cnt + 3'd1;
        rx_byte <= {rx_byte[6:0], pico_data};
    end
end

always @(posedge clk)
    byte_received <= cs_active && sclk_rise && (bit_cnt == 3'd7);

reg led1;
always @(posedge clk)
    if (byte_received) led1 <= ~led1;

assign led2 = rx_byte[0];
assign led3 = rx_byte[1];

reg [7:0] tx_byte;

always @(posedge clk) begin
    if (cs_active) begin
        if (cs_start)
            tx_byte <= ~rx_byte;
        else if (sclk_fall)
            tx_byte <= {tx_byte[6:0], 1'b0};
    end
end

assign SPI_POCI = tx_byte[7];

endmodule
