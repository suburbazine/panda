int default_rx_hook(CAN_FIFOMailBox_TypeDef *to_push) {
  UNUSED(to_push);
  return true;
}

int block = 0;
void send_id(uint8_t fca_cmd_act, uint8_t aeb_cmd_act, uint8_t cf_vsm_warn_fca11, uint8_t cf_vsm_warn_scc12,
             uint8_t obj_valid, uint8_t acc_obj_lat_pos_1, uint8_t acc_obj_lat_pos_2, uint8_t acc_obj_dist_1,
             uint8_t acc_obj_dist_2, uint8_t acc_obj_rel_spd_1, uint8_t acc_obj_rel_spd_2);
void escc_scc11(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_scc12(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_scc13(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_scc14(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_fca11(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_fca12(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3,
                uint8_t bit4, uint8_t bit5, uint8_t bit6, uint8_t bit7);
void escc_frt_radar11(uint8_t bit0, uint8_t bit1);

// *** no output safety mode ***

static void nooutput_init(int16_t param) {
  UNUSED(param);
  controls_allowed = false;
  relay_malfunction_reset();
}

static int nooutput_tx_hook(CAN_FIFOMailBox_TypeDef *to_send) {
  UNUSED(to_send);
  return false;
}

static int nooutput_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  UNUSED(lin_num);
  UNUSED(data);
  UNUSED(len);
  return false;
}

uint8_t fca_cmd_act = 0;
uint8_t aeb_cmd_act = 0;
uint8_t cf_vsm_warn_fca11 = 0;
uint8_t cf_vsm_warn_scc12 = 0;
// Initialize variables to store radar points bytes to send to 2AA
uint8_t obj_valid = 0;
uint8_t acc_obj_lat_pos_1 = 0;
uint8_t acc_obj_lat_pos_2 = 0;
uint8_t acc_obj_dist_1 = 0;
uint8_t acc_obj_dist_2 = 0;
uint8_t acc_obj_rel_spd_1 = 0;
uint8_t acc_obj_rel_spd_2 = 0;
static int default_fwd_hook(int bus_num, CAN_FIFOMailBox_TypeDef *to_fwd) {
  int bus_fwd = -1;
  int addr = GET_ADDR(to_fwd);

  int is_scc_msg = ((addr == 1056) || (addr == 1057) || (addr == 1290) || (addr == 905));  // SCC11 || SCC12 || SCC13 || SCC14
  int is_fca_msg = ((addr == 909) || (addr == 1155));  // FCA11 || FCA12
  int is_frt_radar_msg = (addr == 1186);  // FRT_RADAR11

  if (bus_num == 0) {
    // ESCC is receiving messages from sunnypilot or openpilot
    if (is_scc_msg || is_fca_msg || is_frt_radar_msg) {
      block = 1;
    }
    bus_fwd = 2;
  }
  if (bus_num == 2) {
    // TODO: Do we still need to send a custom messages/ID?
    // SCC11: Forward radar points to 0x2AA
    if (addr == 1056) {
      obj_valid = (GET_BYTE(to_fwd, 2) & 0x1);
      acc_obj_lat_pos_1 = GET_BYTE(to_fwd, 3);
      acc_obj_lat_pos_2 = (GET_BYTE(to_fwd, 4) & 0x1);
      acc_obj_dist_1 = (GET_BYTE(to_fwd, 4) & 0xFE);
      acc_obj_dist_2 = (GET_BYTE(to_fwd, 5) & 0xF);
      acc_obj_rel_spd_1 = (GET_BYTE(to_fwd, 5) & 0xF0);
      acc_obj_rel_spd_2 = GET_BYTE(to_fwd, 6);

      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_scc11(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);
    }
    // SCC12: Detect AEB, override and forward is_scc_msg && is_frt_radar_msg && is_fca_msg
    if (addr == 1057) {
      int aeb_decel_cmd = GET_BYTE(to_fwd, 2);
      int aeb_req = (GET_BYTE(to_fwd, 6) >> 6) & 1U;
      aeb_cmd_act = (GET_BYTE(to_fwd, 6) >> 6) & 1U;
      cf_vsm_warn_scc12 = ((GET_BYTE(to_fwd, 0) >> 4) & 0x2);

      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_scc12(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);

      if ((aeb_decel_cmd != 0) || (aeb_req != 0)) {
        block = 0;
      }
    }
    if (addr == 1290) {
      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_scc13(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);
    }
    if (addr == 905) {
      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_scc14(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);
    }
    // FCA11: Detect FCW, override and forward is_scc_msg && is_frt_radar_msg && is_fca_msg
    if (addr == 909) {
      int CR_VSM_DecCmd = GET_BYTE(to_fwd, 1);
      int FCA_CmdAct = (GET_BYTE(to_fwd, 2) >> 4) & 1U;
      int CF_VSM_DecCmdAct = (GET_BYTE(to_fwd, 3) >> 7) & 1U;
      fca_cmd_act = (GET_BYTE(to_fwd, 2) >> 4) & 1U;
      cf_vsm_warn_fca11 = ((GET_BYTE(to_fwd, 0) >> 2) & 0x2);

      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_fca11(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);

      if ((CR_VSM_DecCmd != 0) || (FCA_CmdAct != 0) || (CF_VSM_DecCmdAct != 0)) {
        block = 0;
      }
    }
    if (addr == 1155) {
      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      uint8_t bit2 = GET_BYTE(to_fwd, 2);
      uint8_t bit3 = GET_BYTE(to_fwd, 3);
      uint8_t bit4 = GET_BYTE(to_fwd, 4);
      uint8_t bit5 = GET_BYTE(to_fwd, 5);
      uint8_t bit6 = GET_BYTE(to_fwd, 6);
      uint8_t bit7 = GET_BYTE(to_fwd, 7);
      escc_fca12(bit0, bit1, bit2, bit3,
                 bit4, bit5, bit6, bit7);
    }
    if (addr == 1186) {
      uint8_t bit0 = GET_BYTE(to_fwd, 0);
      uint8_t bit1 = GET_BYTE(to_fwd, 1);
      escc_frt_radar11(bit0, bit1);
    }
    send_id(fca_cmd_act, aeb_cmd_act, cf_vsm_warn_fca11, cf_vsm_warn_scc12 , obj_valid, acc_obj_lat_pos_1, acc_obj_lat_pos_2, acc_obj_dist_1, acc_obj_dist_2, acc_obj_rel_spd_1, acc_obj_rel_spd_2);
    int block_msg = (block && (is_scc_msg || is_fca_msg || is_frt_radar_msg));
    if (!block_msg) {
      bus_fwd = 0;
    }
  }
  return bus_fwd;
}

const safety_hooks nooutput_hooks = {
  .init = nooutput_init,
  .rx = default_rx_hook,
  .tx = nooutput_tx_hook,
  .tx_lin = nooutput_tx_lin_hook,
  .fwd = default_fwd_hook,
};

// *** all output safety mode ***

static void alloutput_init(int16_t param) {
  UNUSED(param);
  controls_allowed = true;
  relay_malfunction_reset();
}

static int alloutput_tx_hook(CAN_FIFOMailBox_TypeDef *to_send) {
  UNUSED(to_send);
  return true;
}

static int alloutput_tx_lin_hook(int lin_num, uint8_t *data, int len) {
  UNUSED(lin_num);
  UNUSED(data);
  UNUSED(len);
  return true;
}

const safety_hooks alloutput_hooks = {
  .init = alloutput_init,
  .rx = default_rx_hook,
  .tx = alloutput_tx_hook,
  .tx_lin = alloutput_tx_lin_hook,
  .fwd = default_fwd_hook,
};
