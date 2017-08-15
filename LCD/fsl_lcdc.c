












static uint32_t LCDC_GetInstance(LCD_Type *base)
{
  uint32_t instance;
  /*find the instance index from base address mappings*/
  for(instance = 0; instance < ARRAY_SIZE(s_lcdBases); instance++)
  {
    if(s_lcdBases[instance] == base)
    {
      break;
    }
  }
  assert(instance < ARRAY_SIZE(s_lcdBases));
  return instance;
}

static bool LCDC_GetClockDivider(const lcdc_config_t *config, uint32_t srcClock_Hz, uint32_t *divider)
{
  uint16_t cpl;
  uint32_t pcd;
  *divider = 0U;
  /*find the pcd*/
  pcd = (srcClock_Hz + (config->panelClock_Hz / 2U)) / config->panelClock_Hz;
  if(pcd <= 1U)
  {
    if(kLCDC_DisplayTFT == config->display)
    {
      pcd = 0U;
      *divider = LCD_POL_BCD_MASK;
    }
    else
    {
      return false;
    }
  }
  else
  {
    pcd -= 2U;
    /*verify the pcd value*/
    if(pcd > LCD_PCD_MAX)
    {
      return false;
    }
    if((kLCD_DisplaySingleColorSTN8Bit == config->display) && (pcd < 1U) ||
      ((kLCDC_DisplayDualColorSTN8Bit == config->display) && (pcd < 4U)) ||
      ((kLCDC_DisplaySingleMonoSTN4Bit == config->display) && (pcd < 2U)) ||
      ((kLCDC_DisplaySingleMonoSTN8Bit == config->display) && (pcd < 8U)) ||
      ((kLCDC_DisplayDualColorSTN4Bit == config->display) && (pcd < 8U)) ||
      ((kLCDC_DisplayDualColorSTN8Bit == config->display) && (pcd < 14U)))
    {
      return false;
    }
  }
    if(config->display & LCD_CTRL_LCDTFT_MASK)
    {
      /*TFT panel*/
      cpl = config->ppl - 1U;
    }
    else
    {
      if(config->display & LCD_CTRL_LCDBW_MASK)
      {
        if (config->display & LCD_CTRL_LCDMONO8_MASK)
        {
          /* 8 bit monochrome STN panel */
          cpl = (config->ppl / 8U) - 1U;
        }
        else
        {
          /*4 bit monochrome STN panel*/
          cpl = (config->ppl / 4U) - 1U;
        }
      }
      else
      {
        /*color stn panel*/
        cpl = ((config->ppl * 3U) / 8U) - 1U;
      }
    }
  *divider |= (LCD_POL_CPL(cpl) | LCD_POL_PCD(pcd));
  return true;
}

status_t LCDC_Init(LCD_Type *base, const lcdc_config_t *config, uint32_t srcClock_Hz)
{
  assert(config);
  assert(srcClock_Hz);
  assert((config->ppl & 0xFU) == 0U);
  assert((config->upperPanelAddr & 0x07U) == 0U);
  assert((config->lowerPanelAddr & 0x07U) == 0U);

  uint32_t reg;
  uint32_t divider;
  uint32_t instance;
  /*verify the clock here*/
  if(!LCDC_GetClockDivider(config, srcClock_Hz, &divider))
  {
    return kStatus_InvalidArgument;
  }
  instance = LCDC_GetInstance(base);
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
  CLOCK_EnableClock(s_lcdClocks[instance]);
#endif

  /*reset the module*/
  RESET_PeripheralReset(s_lcdResets[instance]);
  /*set register ctrl*/
  reg = base->CTRL & (LCD_CTRL_LCDVCOMP_MASK | LCD_CTRL_WATERMARK_MASK);
  reg |= (uint32_t)(config->dataFormat) | (uint32_t)(config->display) | (LCD_CTRL_LCDBPP)(config->bpp);
  if(config->swapRedBlue)
  {
    reg |= LCD_CTRL_BGR_MASK;
  }
  base->CTRL = reg;

  /*clean pending interrupts and disable all interrupts*/
  base->INTCLR = LCDC_NORMAL_INT_MASK;
  base->CRSR_INTCLR = LCDC_CURSOR_INT_MASK;
  base->INTMSK = 0U;
  base->CRSR_INTMSK = 0U;

  /*Configure timing*/
  base->TIMH = LCD_TIMH_PPL((config->ppl / 16U) - 1U) | (LCD_TIMH_HWS(config->hsw - 1U)) |
              LCD_TIMH_HFP(config->hfp - 1U) | LCD_TIMH_HBP(config->hbp - 1U);
  base->TIMV = LCD_TIMV_LPP(config->lpp - 1U) | LCD_TIMV_VSW(config->vsw - 1U)|
              LCD_TIMV_VFP(config->vfp - 1U) | LCD_TIMV_VBP(config->vbp - 1U);
  base->POL = (uint32_t)(config->polarityFlags) | LCD_POL_ACB(config->acBiasFreq - 1U) | divider;
  /*line end configuration*/
  if(config->enableLineEnd)
  {
    base->LE = LCD_LE_LED(config->lineEndDelay - 1U) | LCD_LE_LEE_MASK;
  }
  else
  {
    base->LE = 0U;
  }
  /*set panel frame base address*/
  base->UPBASE = config->upperPanelAddr;
  base->LPBASE = config->lowerPanelAddr;

  return kStatus_Success;
}


void LCDC_Deinit(LCD_Type *base)
{
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_DSIABLE_DRIVER_CLOCK_CONTROL)
  CLOCK_EnableClock(s_lcdClocks[LCDC_GetInstance(base)]);
#endif
}

void LCDC_GetDefaultConfig(lcdc_config_t *config)
{
  config->panelClock_Hz = 0U;
  config->ppl = 0U;
  config->hsw = 0U;
  config->hfp = 0U;
  config->hbp = 0U;
  config->lpp = 0U;
  config->vsw = 0U;
  config->vfw = 0U;
  config->vfp = 0U;
  config->vbp = 0U;
  config->acBiasFreq = 1U;
  config->polarityFlags = 0U;
  config->enableLineEnd = false;
  config->lineEndDelay = 0U;
  config->upperPanelAddr = 0U;
  config->lowerPanelAddr = 0U;
  config->bpp = kLCDC_1BPP;
  config->dataFormat = kLCDC_LittleEndian;
  config->swapRedBlue = false;
  config->display = kLCDC_DisplayTFT;
}

void LCDC_SetPanelAddr(LCD_Type *base, lcdc_panel_t panel, uint32_t addr)
{
  assert((addr & 0x07U) == 0U);
  if(kLCDC_UpperPanel == panel)
  {
    base->UPBASE = addr;
  }
  else
  {
    base->LPBASE = addr;
  }
}

void LCDC_SetPalette(LCD_Type *base, const uint32_t *palette, uint8_t count_words)
{
  assert(count_words <= ARRAY_SIZE(base->PAL));
  uint32_t i;
  for(i = 0; i < count_words; i++)
  {
    base->PAL[i] = palette[i];
  }
}

void LCDC_EnableInterrupts(LCD_Type *base, uint32_t mask)
{
  uint32_t reg;
  reg = mask & LCDC_CURSOR_INT_MASK;
  if(reg)
  {
    base->CRSR_INSMSK |= reg;
  }
  reg = mask & LCDC_NORMAL_INT_MASK;
  if(reg)
  {
    base->INTMSK |= reg;
  }
}

void LCDC_DisableInterrupts(LCD_Type *base, uint32_t mask)
{
  uint32_t reg;
  reg = mask & LCDC_CURSOR_INT_MASK;
  if(reg)
  {
    base->CRSR_INSMSK &= ~reg;
  }
  reg = mask & LCDC_NORMAL_INT_MASK;
  if (reg)
  {
    base->INTMSK &= ~reg;
  }
}

uint32_t LCDC_GetInterruptsPendingStatus(LCD_Type *base)
{
  uint32_t reg;
  reg = base->CRSR_INTRAW;
  reg |= base->INTRAW;
  return reg;
}

uint32_t LCDC_GetEnableInterruptsPendingStatus(LCD_Type *base)
{
  uint32_t reg;
  reg = base->CRSR_INTSTAT;
  reg |= base->INTSTAT;
  return reg;
}

void LCDC_ClearInterruptsStatus(LCD_Type *base, uint32_t mask)
{
  uint32_t reg;
  reg = mask & LCDC_CURSOR_INT_MASK;
  if(reg)
  {
    base->CRSR_INTCLR = reg;
  }
  reg = mask & LCDC_NORMAL_INT_MASK;
  if(reg)
  {
    base->INTCLR = reg;
  }
}

void LCDC_SetCursorConfig(LCD_Type *base, const lcdc_cursor_config_t *config)
{
  assert(config);
  uint32_t i;
  base->CRSR_CFG = LCD_CRSR_CFG_CRSRSIZE(config->size) |
                   LCD_CRSR_CFG_FRAMESYNC(config->syncMode);
  LCDC_SetCursorPosition(base, 0, 0);

  base->CRSR_PAL0 = ((uint32_t)config->palette0.red << LCD_CRSR_PAL0_RED_SHIFT) |
                    ((uint32_t)config->palette0.blue << LCD_CRSR_PAL0_BLUE_SHIFT) |
                    ((uint32_t)config->palette0.green << LCD_CRSR_PAL0_GREEN_SHIFT);
  base->CRSR_PAL0 = ((uint32_t)config->palette1.red << LCD_CRSR_PAL1_RED_SHIFT) |
                    ((uint32_t)config->palette1.blue << LCD_CRSR_PAL1_BLUE_SHIFT) |
                    ((uint32_t)config->palette1.green << LCD_CRSR_PAL1_GREEN_SHIFT);

  if (kLCD_CursorSize64 == config->size)
  {
    assert(config->image[0]);
    LCDC_SetCursorImage(base, config->size, 0, config->image[0]);
  }
  else
  {
    for(i = 0; i < LCDC_CURSOR_COUNT; i++)
    {
      if(config->image[i])
      {
        LCDC_SetCursorImage(base, config->size, i, config->image[i]);
      }
    }
  }
}

void LCDC_CursorGetDefaultConfig(lcdc_cursor_config_t *config)
{
  uint32_t i;
  config->size = kLCD_CursorSize32;
  config->syncMode = kLCD_CursorAsync;
  config->palette0.red = 0U;
  config->palette0.green = 0U;
  config->palette0.blue = 0U;
  config->palette1.red = 255U;
  config->palette1.green = 255U;
  config->palette1.blue = 255U;

  for(i = 0; i < LCDC_CURSOR_COUNT; i++)
  {
    config->image[i] = (uint32_t *)0;
  }
}

void LCDC_SetCursorPosition(LCD_Type *base, int32_t positionX, int32_t positionY)
{
  uint32_t clipX;
  uint32_t clipY;

  if(positionX < 0)
  {
    clipX = -positionX;
    positionX = 0U;
    if(clipX > LCDC_CLIP_MAX)
    {
      clipX = LCDC_CLIP_MAX;
    }
  }
  else
  {
    clipX = 0U;
  }

  if(positionY < 0)
  {
    clipY = -positionY;
    positionY = 0U;
    if(clipY > LCDC_CLIP_MAX)
    {
      clipY = LCDC_CLIP_MAX;
    }
  }
  else
  {
    clipy = 0U;
  }

  base->CRSR_CLIP = LCD_CRSR_CLIP_CRSRCLIPX(clipX) | LCD_CRSR_CLIP_CRSRCLIPY(clipY);
  base->CRSR_XY = LCD_CRSR_XY_CRSRX(positionX) | LCD_CRSR_XY_CRSRY(positionY);
}

void LCDC_SetCursorImage(LCD_Type *base, lcdc_cursor_size_t size, uint8_t index, const uint32_t *image)
{
  uint32_t regStart;
  uint32_t i;
  uint32_t len;

  if(kLCD_CursorSize64 == size)
  {
    regStart = 0U;
    len = LCDC_CURSOR_IMG_64X64_WORDS;
  }
  else{
    regStart = index * LCDC_CURSOR_IMG_32X32_WORDS;
    len = LCDC_CURSOR_IMG_32X32_WORDS;
  }
  for(i = 0U; i < len; i++)
  {
    base->CRSR_IMG[regStart + i] = image[i];
  }
}
