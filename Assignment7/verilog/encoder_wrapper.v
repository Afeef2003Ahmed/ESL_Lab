module encoder_wrapper #(
    parameter DATA_WIDTH = 32,
    parameter LED_WIDTH = 8
)(
    input  wire        clk,
    input  wire        reset,

    input  wire [2:0]  slave_address,
    input  wire        slave_read,
    output reg  [DATA_WIDTH-1:0] slave_readdata,
    input  wire        slave_write,
    input  wire [DATA_WIDTH-1:0] slave_writedata,
    input  wire [(DATA_WIDTH/8)-1:0]  slave_byteenable,

    input  wire        enc_a, 
    input  wire        enc_b,

    output wire [LED_WIDTH-1:0]  led_out          
);

wire [31:0] encoder_value;
wire encoder_dir;

localparam ADDR_COUNTER = 3'b000;
localparam ADDR_DIR     = 3'b001;

encoder_ip my_ip (
    .clk(clk),
    .rst(reset),
    .enc_a(enc_a),
    .enc_b(enc_b),
    .counter(encoder_value),
    .direction(encoder_dir)
);


assign led_out = encoder_value[LED_WIDTH-1:0];

always @(posedge clk or posedge reset) begin
    if (reset) begin
        slave_readdata <= 32'b0;
    end else if (slave_read) begin
        case (slave_address)
            ADDR_COUNTER: slave_readdata <= encoder_value;
            ADDR_DIR:     slave_readdata <= {31'b0, encoder_dir};
            default:      slave_readdata <= 32'b0;
        endcase
    end
end

endmodule