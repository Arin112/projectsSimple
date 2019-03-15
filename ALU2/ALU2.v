module ALU2
(
	input [7:0] A,
	input [7:0] B,
	input data_enable,
	input [3:0] control,
	input control_enable,
	input clock,
	input reset,
	output reg [15:0] Result,
	output reg result_enable,
	output [15:0] read_res
);
wire [15:0] a = A;
wire [15:0] b = B;

reg [15:0] tA=0;
reg [15:0] tB=0;
//wire [15:0] tResult;
//wire [15:0] tread_res;
wire tEnable;
assign tEnable = result_enable && control_enable;
//reg [3:0] op=0;
mem inst0(.wraddress(control), .rdaddress(control), .q(read_res), .data(Result), .clock(clock), .wren(tEnable));
always @(posedge clock) begin
//	read_res <= tread_res;
	if(reset)begin
		tA<=0;
		tB<=0;
		//op<=0;
		Result<=0;
		result_enable<=0;
	end

	if(data_enable) begin
		tA<=A;
		tB<=B;
	end
	
	if(control_enable && data_enable) begin
		//op<=control;
		result_enable<=1;
		
		case(control)
			4'h0: // сложение
				Result <= a + b ; 
			4'h1: // вычитание
				Result <= a - b ;
			4'h2: // умножение
				Result <= a * b;
			4'h3: // деление
				Result <= a/b;
			4'h4: // побитовый сдвиг влево на 1 бит
				Result <= a<<1;
			4'h5: // побитовый сдвиг вправо на 1 бит
				Result <= a>>1;
			4'h6: // циклический сдвиг влево
				Result <= {a[14:0],a[15]};
			4'h7: // циклический сдвиг вправо
				Result <= {a[0],a[15:1]};
			4'h8: //  побитовое и
				Result <= a & b;
			4'h9: //  побитовое или
				Result <= a | b;
			4'hA: //  побитовое исключающее или 
				Result <= a ^ b;
			4'hB: //  побитовое отрицание побитового исключающего или
				Result <= ~(a | b);
			4'hC: // побитовое отрицание побитового и
				Result <= ~(a & b);
			4'hD: // побитовое отрцание побитового исключающего и
				Result <= ~(a ^ b);
			4'hE: // сравнение больше чем
				Result <= (a>b)?8'd1:8'd0 ;
			4'hF: // сравнение меньше чем
				Result <= (a==b)?8'd1:8'd0 ;
			default: Result <= a + b ;
		endcase
	end else if (control_enable) begin
				result_enable<=1;
		
		case(control)
			4'h0: // сложение
				Result <= tA + tB ; 
			4'h1: // вычитание
				Result <= tA - tB ;
			4'h2: // умножение
				Result <= tA * tB;
			4'h3: // деление
				Result <= tA/tB;
			4'h4: // побитовый сдвиг влево на 1 бит
				Result <= tA<<1;
			4'h5: // побитовый сдвиг вправо на 1 бит
				Result <= tA>>1;
			4'h6: // циклический сдвиг влево
				Result <= {tA[14:0],tA[15]};
			4'h7: // циклический сдвиг вправо
				Result <= {tA[0],tA[15:1]};
			4'h8: //  побитовое и
				Result <= tA & tB;
			4'h9: //  побитовое или
				Result <= tA | tB;
			4'hA: //  побитовое исключающее или 
				Result <= tA ^ tB;
			4'hB: //  побитовое отрицание побитового исключающего или
				Result <= ~(tA | tB);
			4'hC: // побитовое отрицание побитового и
				Result <= ~(tA & tB);
			4'hD: // побитовое отрцание побитового исключающего и
				Result <= ~(tA ^ tB);
			4'hE: // сравнение больше чем
				Result <= (tA>tB)?8'd1:8'd0 ;
			4'hF: // сравнение меньше чем
				Result <= (tA==tB)?8'd1:8'd0 ;
			default: Result <= tA + tB ;
		endcase
	end else begin
		result_enable <= 0;
	end
	
end

endmodule
