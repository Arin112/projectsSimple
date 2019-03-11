module stack
(
	input [7:0] data,
	input push, pop, clk,
	output [7:0] q
);
	reg [7:0] mem [15:0];
	reg [3:0] addr=4'h0;
	
	assign q = mem[addr-4'h1];
	
	always @ (posedge clk) begin
		
		if (push)
		begin
			addr <= addr + 4'h1;
			mem[addr] <= data;
		end
		
		if(pop)
			addr <= addr - 4'h1;
	end

endmodule
