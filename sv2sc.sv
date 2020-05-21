`timescale 1ns/1ns;
`include "uvm_macros.svh"
import uvm_pkg::*;
import uvmc_pkg::*;
`include "apb_host.svh";

module sv_main;
  
  apb_host apb_h = new("apb_h");
  
  initial begin
    
    uvmc_tlm#()::connect(apb_h.initSocket, "foo");
    
    run_test();
	
  end
endmodule
