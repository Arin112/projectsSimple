module ClkDiv
 
(
input clk,
output clk_out,
input [7:0] div
);

wire isOdd = div & 8'h01;

wire [7:0] N;
assign N = div;
reg [7:0] pos_count, neg_count;

wire [7:0] eN;
assign eN = div>>1;
reg [7:0] r_reg;
wire [7:0] r_nxt;
reg clk_track;
assign r_nxt = r_reg+8'h01;  
always @(posedge clk) begin
	if(isOdd)begin // optional, reduces the noise
		if (pos_count == N-1)
			pos_count <= 0;
		else
			pos_count<= pos_count + 8'h01;
	end else begin
		if (r_nxt == eN)
			begin
				r_reg <= 0;
				clk_track <= ~clk_track;
			end
		else
			r_reg <= r_nxt;
	end
end

always @(negedge clk)
	if(isOdd) begin // optional, reduces the noise
		if (neg_count == N-1)
			neg_count <= 0;
		else
			neg_count<= neg_count + 8'h01;
	end

assign clk_out = (((pos_count > (N>>1)) | (neg_count > (N>>1))) & isOdd) || ( (clk_track) &(~isOdd) ); 
endmodule