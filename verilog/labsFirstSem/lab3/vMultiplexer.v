module vMultiplexer
(
input [3:0] coord, // координата - который из входных сигналов выбрать
// по требованию задания у модуля должно быть 16 восьмибитных входных сигналов
input [7:0] q1,
input [7:0] q2,
input [7:0] q3,
input [7:0] q4,
input [7:0] q5,
input [7:0] q6,
input [7:0] q7,
input [7:0] q8,
input [7:0] q9,
input [7:0] q10,
input [7:0] q11,
input [7:0] q12,
input [7:0] q13,
input [7:0] q14,
input [7:0] q15,
input [7:0] q16,
output [7:0] q
);

wire [7:0] qs [15:0];

assign qs[0] = q1;
assign qs[1] = q2;
assign qs[2] = q3;
assign qs[3] = q4;
assign qs[4] = q5;
assign qs[5] = q6;
assign qs[6] = q7;
assign qs[7] = q8;
assign qs[8] = q9;
assign qs[9] = q10;
assign qs[10] = q11;
assign qs[11] = q12;
assign qs[12] = q13;
assign qs[13] = q14;
assign qs[14] = q15;
assign qs[15] = q16;

assign q = qs[coord];

endmodule 
            