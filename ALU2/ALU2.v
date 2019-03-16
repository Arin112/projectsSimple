module ALU2
/*
Логика модуля следующая.
Если data_enable==1 и control_enable == 1,
то result_enable==1 и Result содержит результат операции над А и B (в тот же тик).
Если Если data_enable==0 и control_enable == 1,
то result_enable==1 и Result содержит результат операции над последними A и B, когда data_enable был 1 (в тот же тик).
Если control_enable == 1, то result_enable == 0.

read_res содержит результат последней операции типа control (с задержкой в 1 тик)
*/
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

reg [15:0] tA=0; // Регистры для хранения последних данных с data_enable==1
reg [15:0] tB=0;

reg [3:0] addr=0;	// обеспечивает задержку при записи в память
						// в память записывается Result, если не обеспечить
						// задержку как у Result, то запись будет происходить по неверному адресу

wire [15:0] a = data_enable? A : tA; // работает как assign
wire [15:0] b = data_enable? B : tB; // так осуществляется выбор между текущеми и запомненными данными

mem inst0(.address(addr), .q(read_res), .data(Result),
	.inclock(~clock), .outclock(clock), .we(result_enable));
// Входной клок работает по отрицанию основного для обеспечения
// минимально возможной задержки в 1 тик перед выдачей данных,
// так как надо сначала подождать, пока в Result будет положено значение
// с последнего расчёта, а только потом писать в память.
// Можно обойтись без этого, если все вычисления проводить в wire,
// что вроде как ломает первоначальный замысел лабы

always @(posedge clock) begin
	addr<=control;// по тику запоминаем последнюю операцию
	
	if(reset)begin // по ресету сбрасываем все регистры в этом блоке
		tA<=0;
		tB<=0;
		Result<=0;
		result_enable<=0;
		addr<=0;
	end

	if(data_enable) begin // запоминаем последние данные
		tA<=A;
		tB<=B;
	end
	
	if(control_enable) begin // производим расчёт значения
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
	end else begin // если control_enable==0, то и result_enable==0
		result_enable <= 0;
	end
	
end

endmodule
