﻿module vSevenSegment
(
input [3:0] x, // число, которое надо вывести на семисегментном индикаторе
output [6:0] z // вывод для семисегментного индикатора
);
/* так семисегментный индикатор должен расценивать выходные биты
      z[0]
     ------ 
z[5]|      | z[1]
    | z[6] |
    |------|
z[4]|      | z[2]
    |      |
     ------ 
      z[3]
*/
wire [3:0] n; // not x

assign n = ~x;

assign z[0] = !((n[3]&n[2]&n[1]&x[0]) | (x[2]&n[1]&n[0]));
assign z[1] = !((x[2]&n[1]&x[0]) | (x[2]&x[1]&n[0]));
assign z[2] = !(n[3]&n[2]&x[1]&n[0]);
assign z[3] = !((n[3]&n[2]&n[1]&x[0]) | (x[2]&n[1]&n[0]) | (x[2]&x[1]&x[0]));
assign z[4] = !((x[2]&n[1]) | x[0]);
assign z[5] = !((n[3]&n[2]&x[0]) | (n[2]&x[1]) | (x[1]&x[0]));
assign z[6] = !((n[3]&n[2]&n[1]) | (x[2]&x[1]&x[0]));

endmodule
