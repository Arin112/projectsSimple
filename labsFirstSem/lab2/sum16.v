module sum16 // то же самое, что и sum4, но в sum4 в качестве аргумента передаются диапазоны
(
input [15:0] a,
input [15:0] b,
input cin,
output [15:0] c,
output cout
);
wire [3:0] couts;
sum4 inst0(.a(a[3:0]), .b(b[3:0]), .cin(cin), .c(c[3:0]), .cout(couts[0]));

genvar x;
generate
	for(x = 1; x <4; x = x + 1)begin : _gen1
		sum4 insts(.a(a[x*4+3:x*4]), .b(b[x*4+3:x*4]), .c(c[x*4+3:x*4]), .cin(couts[x-1]), .cout(couts[x]));
	end
endgenerate
assign cout = couts[3];

endmodule
