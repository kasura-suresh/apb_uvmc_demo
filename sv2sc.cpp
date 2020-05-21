#include "uvmc.h"
using namespace uvmc;

#include "apb_protocol.h"

int sc_main( int argc, char* argv[] )
{
  APB_Master apb_m("apb_m");
  APB_Slave  apb_s("apb_s");
  apb_m.m_port(apb_s.s_export);
  uvmc_connect (apb_m.host_sock,"foo");
  sc_start(-1);
  return 0;
}


