`timescale 1ns / 100ps

module test_crypto();

//declare the signal
	reg clk;
    reg rst_n;
    reg we;
    reg re;
    reg [7:0] addr;
    reg [31:0] wdata;
    wire [31:0] rdata;
	 
	parameter CLK_CYCLE = 10;
	//DUT
	
	crypto dut(
	clk,
    rst_n,
    we,
    re,
    addr,
    wdata,
    rdata
	);
	
	//clk generation
	
	initial clk =1'b0;
	always #(CLK_CYCLE/2) clk = ~clk;
	

	/*task       task_id ;
    port_declaration ;
    procedural_statement ;
endtask
	*/
	
	task bus_write(input [7:0] a, input [31:0] d);
		begin
			@(posedge clk);
			addr <=a;
			we <=1;
			wdata <= d;
			@(posedge clk);
			we <= 0;
		end
	endtask
	
	
	
	//task: read value
	task bus_read(input [7:0] a);
		begin
			@(posedge clk);
			addr <= a;
			re <= 1;
			@(posedge clk);
			$display("Read from Addr %h: Data = %h", a, rdata);
			re <= 0;
		end
	endtask
	
	initial begin
	//inizialization
	rst_n = 0; we = 0; re = 0; addr = 0; wdata = 0;
	
	#20 rst_n =1;
	
	//write Data
	bus_write(8'h04, 32'hAAAA_5555);
	//write key
	bus_write(8'h08, 32'h1234_5678);
	
	//enable calculation
	bus_write(8'h00, 32'h0000_0001);
	
	//wait a clk for write process
	@(posedge clk);
	
	//read_process
	bus_read(8'h00);
	
	bus_read(8'h04);
	
	bus_read(8'h08);
	
	bus_read(8'h0C);
	
	//control if it is correct
	if (rdata == (32'hAAAA_5555 ^ 32'h1234_5678)) 
    $display("SUCCESS: XOR Result matches!");
else 
    $display("ERROR: Result mismatch! Expected %h, Got %h", (32'hAAAA_5555 ^ 32'h1234_5678), rdata);
	
	//clean reg_result
	bus_write(8'h00, 32'h0000_0002);
	
	//wait a clk for write process
	@(posedge clk);
	
	#100;
	$display("Simulation Finished!");
	$finish;
	end
	
endmodule //test_crypto
	




