﻿module vDiv3and5
// Вообще говоря под делением частоты интернет подразумевает не то, что было дано на лабораторных работах.
// Поэтому назвать данную программу делителем частоты можно только с оговорками.
// Первое - длинны положительного и отрицательного фронтов отличаются.
// Второе - даже если бы длины фронтов совпадали бы, эти делители бы были названы
// делителями на 6 и на 10, так как отсчёт обычно ведётся по обоим фронтам.
// Т.е. не частота положительного фронта, а частота смены сигнала.
// Тем не менее назвать данное определение неверным нельзя,
// ведь формально период между повторениями положительных фронтов будет
// действительно домножен на 3 и на 5 соответственно.
(
input clk,
output reg div3,
output reg div5
);
	reg [1:0] cnt3 = 0;
	reg [2:0] cnt5 = 0;
	
	always @(posedge clk) begin
		cnt3 <= (cnt3 < 2)? cnt3 + 2'h1 : 2'h0;
		cnt5 <= (cnt5 < 4)? cnt5 + 3'h1 : 2'h0;
		div3 <= (cnt3==1)? 1'h1 : 1'h0;
		div5 <= (cnt5==3)? 1'h1 : 1'h0;
	end

endmodule
