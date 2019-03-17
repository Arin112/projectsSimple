module print
(
input clk, 					// clock
input write, 				// start write mem data to out pin
input init, 				// write init data to mem on init bit
input [7:0] memData, 		// read form mem
output reg [7:0] iniData, 	// write init data to mem
output reg enWrite, 		// enable write to mem bit
output reg [4:0] addr, 		// addr of curren read|write pos
output reg out				// print mem data
//output reg [1:0] state, 	// for debug
//output reg [5:0] curAddr	// for debug
);
initial begin
	iniData = 0;
	enWrite = 0;
	addr = 0;
	out = 0;
//	state = 0; // for debug
//	curAddr = 0; // for debug
end
reg [1:0] state = 0;
// state == 0 - wait for command
// state == 1 - write init data to mem
// state == 2 - write data to out pin

reg [5:0] curAddr = 0;

reg [7:0] dataToWrite;
reg [4:0] curBit = 0;
reg ready = 0;
reg sum = 0;

// main
always @(posedge clk) begin
	if(state == 0)begin
		if(init == 1) state <= 1;
		else if(write == 1) state <= 2;
	end
	if(state == 1) begin
		enWrite <= 1;
		iniData <= curAddr + 1; // first number in mem will be zero
		curAddr <= curAddr + 1;
		addr <= curAddr;
		if(curAddr == 31)begin
			enWrite <= 0;
			state <= 0;
			out <= 1;
		end
	end else
	if(state == 2) begin
		out <= 1;
		
		if(ready)begin
			if(curBit == 0)begin
				dataToWrite <= memData;
				curBit <= curBit + 1;
				out <= 0;
				sum <= 0;
			end else if (curBit >= 1 && curBit <= 8) begin
				out <= dataToWrite[7-(curBit-1)];
				curBit <= curBit + 1;
				sum <= sum ^ dataToWrite[curBit-1];
			end else if (curBit == 9) begin
				out <= sum;
				curBit <= curBit + 1;
			end else if(curBit == 10) begin
				out <= 1;
				curBit <= curBit + 1;
			end else if (addr < 31) begin
				out <= 1;
				curBit <= 0;
				addr <= addr + 1;
			end else begin
				addr <= 0;
				state <= 0;
			end
		end else begin
			curBit <= 0;
			dataToWrite <= 0;
			addr <= 0;
			ready <= 1;
		end
	end
end

endmodule
