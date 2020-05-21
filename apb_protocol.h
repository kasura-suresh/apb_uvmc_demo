#include<list>
#include<utility>
#include<systemc.h>
#include <tlm>
#include<iostream>
using namespace std;
using namespace tlm;

#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
using namespace tlm_utils;

#define LOG() cout << "@ " << sc_time_stamp() << ":: " << name() << ": " \
            << "[" << __func__ << "]: "

// There will be SV-UVM Host
// This UVM Host will send data to SC Host, which in turn will send 
// the data to Master to start the APB transactions

// This is the SystemC intermediate host
SC_MODULE(Host) {
  // simple_target_socket<Host>  uvmc_tlm_sock;
  simple_initiator_socket<Host>  master_sock;

  SC_CTOR(Host) : master_sock("master_sock") {
    LOG() << "Inside ctor ...";
    // master_sock.register_b_transport(this, &Host::b_transport);

    // Add send_transactions thread
    SC_THREAD(send_transactions);
  }

  void b_transport(tlm_generic_payload& gp, sc_time& delay) {
    master_sock->b_transport(gp, delay);
  }

#if 1
  tlm_generic_payload* make_payload(uint64_t addr, tlm_command cmd) {
    tlm_generic_payload* gp = new tlm_generic_payload;
    gp->set_address(addr);
    gp->set_command(cmd);
    gp->set_data_length(1);
    gp->set_data_ptr(new unsigned char[gp->get_data_length()]);
    gp->get_data_ptr()[0] = rand() % 10;
    return gp;
  }

  // Thread to send all the testcases
  void send_transactions() {
    LOG() << "Started ...";
    int num_trans = 10;
    sc_time delay(10, SC_NS);
    int i=0;
    int adr = 1;
    tlm_generic_payload* gptr;
    // Do all the write followed by read transaction to same address
    while (i < num_trans) {
      // Create payload and do write transactions
      gptr = make_payload(adr, TLM_WRITE_COMMAND);
      LOG() << "Host calling WRITE addr: " << gptr->get_address() << ", data: " << gptr->get_data_ptr()[0] << endl;
      b_transport(*gptr, delay);

      // Reset payload data and do read transactions
      gptr->set_read();
      gptr->get_data_ptr()[0] = 0;
      b_transport(*gptr, delay);
      LOG() << "Host calling READ addr: " << gptr->get_address() << ", data: " << gptr->get_data_ptr()[0] << endl;

      i++;
      adr++;
    }
  }
#endif
};

struct APB_Interface : public sc_interface
{
  virtual void do_write( unsigned int addr ,unsigned int data) = 0 ;
  virtual unsigned int do_read ( unsigned int addr) = 0 ;
};

SC_MODULE (APB_Master)
{
  simple_target_socket<APB_Master> host_sock;
  // sc_in <bool> clk;
  sc_port <APB_Interface> m_port;
  // sc_in<int> prdata; 
  // sc_out<int> pwdata;
  
  void b_transport(tlm_generic_payload& gp, sc_time& delay) {
    if (gp.is_write()) {
      LOG() << "Master writing at addr: " << hex << gp.get_address() << ", data: " << *(unsigned int*)gp.get_data_ptr() << endl;
      m_port->do_write(gp.get_address(), *(unsigned int*)gp.get_data_ptr());
    } else {
      LOG() << "Master reading at addr: " << gp.get_address();
      unsigned int read_data = m_port->do_read(gp.get_address());
      *(unsigned int*)gp.get_data_ptr() = read_data;
      LOG() << "\t\t, data: " << hex << *(unsigned int*)gp.get_data_ptr() << endl;
    }
  }

  /*
  void behaviour()
  {

    cout<<"Write operation is ON\n "<<endl;
    for (int i=1; i<=10; i++)
    {
      cout<<"In Master "<<endl ;
      cout<<"address sent by master is "<<i<<endl;
      cout<<"data sent by master is "<<i*2<<endl;
      m_port->do_write(i,i*2);
    }
    cout<<"\n"<<endl;
    cout<<"Read operation is ON\n "<<endl;
    for (int i=1; i<=10; i++)
    {
      cout<<"Master wants to read from the address "<<i<<endl;
      prdata = m_port->do_read(i);
      // wait();
    }
      
  }
  void receive()
  {
    cout<<" data received by master is "<<prdata<<endl;
  }
  */
  SC_CTOR(APB_Master) : host_sock("host_sock")
  {
    host_sock.register_b_transport(this, &APB_Master::b_transport);
    // SC_METHOD(receive);
    // sensitive<<prdata;
    // dont_initialize();
    // SC_THREAD(behaviour);
    // sensitive<<clk;
  }
};


SC_MODULE(APB_Slave), public APB_Interface
{
  // sc_in <bool> clk;
  sc_export<APB_Interface> s_export;
  // list< pair<int,int> >   writeList ;
  // list <int> readList ;
  // pair <int,int> temp ;
  // sc_out<int> prdata  ;
  // sc_in<int> pwdata   ;

  map<unsigned int, unsigned int> mem_data;
  
  void do_write( unsigned int addr ,unsigned int data)
  {
    cout<<"In Slave "<<endl ;
    cout<<"address received  by slave is "<<addr<<endl;
    cout<<"data received  by slave is "<<data<<endl;
    // writeList.push_back(make_pair(addr,data)) ;
    // readList.push_back(addr);
    mem_data[addr] = data;
  }
  unsigned int do_read ( unsigned int addr)
  {
    // if(addr == readList.front())
    // temp = writeList.front();
    // prdata = temp.second ;
    // writeList.pop_front(); 
    // readList.pop_front(); //resizes the list
    if (mem_data.find(addr) == mem_data.end()) { // No data written for this addr
      LOG() << "WARNING: No entry for addr: " << addr << endl;
      return (unsigned)-1;
    } else {
      LOG() << "addr: " << addr << ", data: " << mem_data[addr] << endl;
      return mem_data[addr];
    }
  }
  
  SC_CTOR(APB_Slave)
  {
    s_export.bind(*this);
    // host_sock.register_b_transport(this, &APB_Master::b_transport);
  }
};

#if 0
int sc_main(int argc ,char* argv[])
{
  Host host("host");
  APB_Master m1("m1");
  APB_Slave  s1("s1");
  // sc_clock clk("clk" ,2 ,SC_NS) ;
  
  // sc_signal <int> pwdata,prdata;
  host.master_sock(m1.host_sock);
  
  m1.m_port(s1.s_export);
  // m1.clk(clk);
  // m1.pwdata(pwdata);
  // m1.prdata(prdata);
  
  // s1.clk(clk);
  // s1.pwdata(pwdata);
  // s1.prdata(prdata);
  
  
  sc_start(200,SC_NS) ;
}
#endif
  
  
