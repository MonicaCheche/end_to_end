module crypto(
    input wire clk,
    input wire rst_n,
    input wire we,
    input wire re,
    input wire [7:0] addr,
    input wire [31:0] wdata,
    output reg [31:0] rdata
);

    reg [31:0] ctrl;
    reg [31:0] data;
    reg [31:0] key;
    reg [31:0] result;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ctrl   <= 0;
            data   <= 0;
            key    <= 0;
            result <= 0;
        end
        else begin

            // ---------------- write ----------------
            if (we) begin
                case(addr)
                    8'h00: ctrl <= wdata;
                    8'h04: data <= wdata;
                    8'h08: key  <= wdata;
                endcase
            end

            // ---------------- compute ----------------
            if (ctrl[1]) begin
                result <= 0;
                ctrl[1] <= 0;   // optional auto-clear reset bit
            end
            else if (ctrl[0]) begin
                result <= key ^ data;
            end

        end
    end

    // ---------------- read mux ----------------
    always @(*) begin
        case(addr)
            8'h00: rdata = ctrl;
            8'h04: rdata = data;
            8'h08: rdata = key;
            8'h0C: rdata = result;
            default: rdata = 32'h0;
        endcase
    end

endmodule