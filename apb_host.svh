// import uvm_pkg::*;
// `include "uvm_macros.svh"


class apb_host extends uvm_component;
  `uvm_component_utils(apb_host) // registering the calss with uvm factory

  // declaring b_initiator scocket object 
  uvm_tlm_b_initiator_socket #(uvm_tlm_gp) initSocket; 
  
  function new(string name = "apb_host" , uvm_component parent = null);
      super.new(name,parent);
	  initSocket = new("initSocket" , this);// initialising the initSocket
  endfunction
  
  task run_phase(uvm_phase phase);
       
	   // Allocating the gp
	  uvm_tlm_gp gp = uvm_tlm_gp::type_id::create("gp",this);
      uvm_tlm_time delay = new("delay",1e-12);
	  int num_trans = 10;
      bit[63:0] write_addr[] = new[num_trans];
	  int i = 0;
	  
	  phase.raise_objection(this);
	  repeat(num_trans) begin  
	     
		 delay.set_abstime(1ns,1e-9);
		 assert(gp.randomize() with { gp.m_address inside {[1:10]};
									  gp.m_byte_enable_length == 0;
									  gp.m_length == 4;
									  gp.m_data.size() == m_length;
									  // gp.m_command inside {UVM_TLM_READ_COMMAND ,UVM_TLM_WRITE_COMMAND};
									  gp.m_command == UVM_TLM_WRITE_COMMAND;
									 });
									  
		 `uvm_info("APB_HOST/PKT",{"\n",gp.sprint()},UVM_MEDIUM)							  
									 
	     //sending the commands to apb_master
	     `uvm_info("INFO","...SV_APB_HOST SENDING TO APB_MASTER...",UVM_MEDIUM)
		 
		 initSocket.b_transport(gp ,delay);

         write_addr[i] = gp.m_address;
         i++;
		 
		end

      // Do read operation on all addresses where write was done
      i = 0;
	  repeat(num_trans) begin  
        
        gp.m_address = write_addr[i];
        gp.m_command = UVM_TLM_READ_COMMAND;
	     //sending the commands to apb_master
	     `uvm_info("INFO","...SV_APB_HOST SENDING READ TO APB_MASTER...",UVM_MEDIUM)
		 initSocket.b_transport(gp ,delay);
         i++;
       end
		
		phase.drop_objection(this);
		
	endtask
endclass
