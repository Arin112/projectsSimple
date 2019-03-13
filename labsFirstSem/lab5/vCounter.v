﻿module vCounter
(
input clk,
output reg [15:0] q
);

	always @(posedge clk)begin
		q <= q + 16'h1 ;
		// 16'h1 - шеснадцатиразрядная единица; можно написать просто 1, но тогда
		// будет использована тридцатидвухразрядная единица и квартус будет
		// выдавать warning на потерю бит
		// h - hex - использованная система счисления
	end

endmodule
