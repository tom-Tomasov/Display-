// ================= PAC-MAN  =================
// ================================================================
//                         PAC-MAN 5x5
// ================================================================
//
//  Pac-Man se zobrazuje pouze v case:
//      08:45 - 08:59
//      19:00 - 19:15
//
//  Jede stale tam a zpet.
//  Cestou ji tecky.
//  Animuje pusu.
//  Display je ROTATE 90, proto bitmapu otacime.
// ================================================================

int pacmanX = 0;
int pacmanSmer = 1;
bool pacmanPusa = false;

unsigned long pacmanAnimace = 0;

bool pacmanAktivni = false;
int pacmanPosledniCas = -1;

// 32 pozic tecek
bool pacmanTecky[32];

// ---------------------------------------------------------------
// Je prave cas na PAC-MANA?
// ---------------------------------------------------------------
bool jeCasPacman()
{
  // rano
  if (h == 8 && m >= 40 && m <= 44)
    return true;

//  if (h == 16 && m >= 0 && m <= 59)
 //   return true;

  if (h == 10 && m >= 0 && m <= 15)
    return true;

  // vecer
  if (h == 18 && m >= 50 && m <= 59)
    return true;

  return false;
}


// ---------------------------------------------------------------
// Zacatek noveho kola
// ---------------------------------------------------------------
void startPacman()
{
  pacmanX = 0;
  pacmanSmer = 1;
  pacmanPusa = false;

  // VZDY znovu vytvorit vsechny tecky
  for (int i = 0; i < 32; i++)
    pacmanTecky[i] = false;

  pacmanAnimace = millis();

  pacmanAktivni = true;
}


// ---------------------------------------------------------------
// PAC-MAN
// ---------------------------------------------------------------
void drawPacman()
{
  unsigned long ted = millis();

  // -------------------------------------------------------------
  // NOVY START PAC-MANA KAZDOU MINUTU
  // -------------------------------------------------------------
  int aktualniCas = h * 60 + m;

  if (aktualniCas != pacmanPosledniCas)
  {
    pacmanPosledniCas = aktualniCas;
    startPacman();
  }
 
 

  // ------------------------Pohyb Pac-Mana-------------------------------------
  if (ted - pacmanAnimace >= 120)
  {
    pacmanAnimace = ted;

    // fyzicky se pohybuje opacnym smerem
    pacmanX -= pacmanSmer;

    // snedeni tecky
    if (pacmanX >= 0 && pacmanX < 32)
      pacmanTecky[pacmanX] = true;

    // jeden konec
    if (pacmanX <= 0)
    {
      pacmanX = 0;
      pacmanSmer = -1;
    }

    // druhy konec
    if (pacmanX >= 27)
    {
      pacmanX = 27;
      pacmanSmer = 1;
    }

    // animace pusy
    pacmanPusa = !pacmanPusa;
  }


  // =============================================================
  //             PAC-MAN 5x5 BITMAPA
  // =============================================================

  int px = pacmanX;

  if (px < 0)
    px = 0;

  if (px > 27)
    px = 27;


  byte source[5];


  if (pacmanPusa)
  {
    source[0] = B01110;
    source[1] = B11111;
    source[2] = B11000;
    source[3] = B11111;
    source[4] = B01110;
  }
  else
  {
    source[0] = B01110;
    source[1] = B11111;
    source[2] = B11111;
    source[3] = B11111;
    source[4] = B01110;
  }


  // =============================================================
  // OTOCENI BITMAPY O 90°
  // =============================================================

  byte rotated[5] = {0, 0, 0, 0, 0};

  for (int x = 0; x < 5; x++)
  {
    for (int y = 0; y < 5; y++)
    {
      if (source[x] & (1 << y))
      {
        int newX = y;
        int newY = 4 - x;

        rotated[newX] |= (1 << newY);
      }
    }
  }


  // =============================================================
  // KRESLENI TECEK
  // =============================================================

  for (int x = 1; x < 28; x += 2)
  {
    if (!pacmanTecky[x])
    {
      if (x >= 0 && x < LINE_WIDTH)
      {
        scr[64 + x] |= B00010000;
      }
    }
  }
  // =============================================================
  // KRESLENI PAC-MANA
  // =============================================================

  if (pacmanSmer == 1)
  {
    for (int i = 0; i < 5; i++)
    {
      if (px + i >= 0 && px + i < LINE_WIDTH)
        scr[64 + px + i] |= (rotated[i] << 2);
    }
  }
  else
  {
    for (int i = 0; i < 5; i++)
    {
      int col = 4 - i;

      if (px + i >= 0 && px + i < LINE_WIDTH)
        scr[64 + px + i] |= (rotated[col] << 2);
    }
  }
}
