////////////////////////////////////////////////////////////////
//devoir3.cpp
//Créé par: Sara Séguin
//Date: 5 novembre 2025
//CE FICHIER N'EST PAS EXHAUSTIF, IL DONNE SIMPLEMENT DES EXEMPLES ET UNE CERTAINE STRUCTURE.À Vous de modifier.
////////////////////////////////////////////////////////////////

#include <climits>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <bitset>
#include <fstream>
#include <string>

#define PAGE_t 256 //Taille d'une page (256 bytes)
#define TAILLE_MEMOIRE_PHYSIQUE 256 * PAGE_t
#define TLB_SIZE 16

////////////////////////////////////////////////////////////////
//Cette fonction retourne la valeur du byte signé
//Créé par: 
//Date: 
//Modifié par:
//Description:
////////////////////////////////////////////////////////////////
int fct_SignedByte(int page, int offset)
{
	//Ouvrir le fichier binaire
  std::ifstream fichier("simuleDisque.bin", std::ios::binary);

	unsigned int LENGTH = 1;
  char bytes;
	
	fichier.seekg(page * PAGE_t + offset);
	fichier.read(&bytes, LENGTH);

	//Fermer le fichier
  fichier.close();

	//Retourner la valeur du byte signé
  return int(bytes);
}

////////////////////////////////////////////////////////////////
//Cette fonction créé un masque afin de lire les bits nécessaires. NE PAS MODIFIER ET UTILISER TEL QUEL DANS LE MAIN
//Créé par: Sara Séguin
//Modifié par:
//Description:
////////////////////////////////////////////////////////////////
unsigned createMask(unsigned a, unsigned b)
{

	unsigned r = 0;
	for(unsigned i=a;i<=b;i++)
	{
		r |= 1 << i;
	}
	return r;
}

// Custom function
void affichageResultats(
    const std::vector<int>& adresseLogique,
    const int adressePhysique[], 
    const char memPhysique[],
    int pageFaultCount,
    int TLBCount,
    int totalAdresses
) 
{
    std::ofstream out("resultats.txt");

    for (int i = 0; i < adresseLogique.size(); i++) {
        int value = (char)memPhysique[adressePhysique[i]];
        out << "Virtual address: " << adresseLogique[i]
            << " Physical address: " << adressePhysique[i]
            << " Value: " << value
            << " Value bin: " << std::bitset<8>(memPhysique[adressePhysique[i]])
            << std::endl;
    }

    // Stats
    std::cout << "=== Stats === \n";
    std::cout << "Page fault: " << ( (double)pageFaultCount / totalAdresses ) * 100 << "%\n";
    std::cout << "TLB hit: " << ( (double)TLBCount / totalAdresses ) * 100 << "%\n";
}

int chargerPageDansFrame(int page, int frame, char memPhysique[TAILLE_MEMOIRE_PHYSIQUE]){
    std::ifstream fichier("simuleDisque.bin", std::ios::binary);
    int physical_addresse = frame * PAGE_t;

    fichier.seekg(page * PAGE_t);
    fichier.read(&memPhysique[physical_addresse], PAGE_t);
    fichier.close();

    return physical_addresse;
}

// TLB
struct TLBEntry {
    int page;
    int frame;
    int lastUse = -1;
};

int findMaxUseTLB(TLBEntry TLB[TLB_SIZE])
{
  int max = -1;
  for(int i = 0; i < TLB_SIZE; i++)
  {
    if(TLB[i].lastUse > max)
    {
      max = TLB[i].lastUse;
    }
  }
  return max;
}

int findMinUseTLB(TLBEntry TLB[TLB_SIZE])
{
  int min = INT_MAX;
  for(int i = 0; i < TLB_SIZE; i++)
  {
    if(TLB[i].lastUse < min)
    {
      min = TLB[i].lastUse;
    }
  }
  return min;
}

int findFrameTLB(TLBEntry TLB[TLB_SIZE], int page)
{
  int max = findMaxUseTLB(TLB);
  int frame = -1;

  for(int i = 0; i < TLB_SIZE; i++)
  {
    if(TLB[i].page == page)
    {
      TLB[i].lastUse = max + 1;
      frame = TLB[i].frame;
      std::cout << "TLB hit: " << page << " est dans TLB: " << frame << std::endl;
      break;
    }
  }
  return frame;
}

void addToTLB(TLBEntry TLB[TLB_SIZE], int page, int frame)
{

  int max = findMaxUseTLB(TLB);
  int min = findMinUseTLB(TLB);

  for(int i = 0; i < TLB_SIZE; i++)
  {
    if(TLB[i].lastUse == min)
    {
      int lastUse = max + 1;
      TLB[i] = {page, frame, lastUse};
      break;
    }
  }
}

//////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// PROGRAMME PRINCIPAL
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
int main()
{
	// 1. Initialisation et déclarations
	char memPhysique[TAILLE_MEMOIRE_PHYSIQUE] = {0};
	int adressePhysique[1000] = {0};
	int tablePage[256][2]={0};
	std::vector<int>adresseLogique;
  TLBEntry TLB[TLB_SIZE] = {};
	
	// 2. Lire le fichier d'adresses à traduire
  std::ifstream fichier("addresses.txt");
  int valeur;
  while (fichier >> valeur) {
    adresseLogique.push_back(valeur);
  }

	// 3. Stocker les pages et offset des addresses dans un vector
	std::vector<int>bits_offset,bits_page;
	
	unsigned r = 0;
	r = createMask(0,7);
	unsigned r2 = 0;
	r2 = createMask(8,15);
	
	for(int i =0; i< adresseLogique.size() ; i++)
	{
		int A = adresseLogique[i];
		int offset = r & A;
		int page = (r2 & A) >> 8;

		bits_offset.push_back(offset);
		bits_page.push_back(page);
	}

  // 4. Charger les adresses physiques
  int indexFrame = 0;
  int pageFaultCount = 0;
  int TLBCount = 0;

	for(int i=0;i<bits_page.size();i++)
	{
    int page = bits_page[i];
    int offset = bits_offset[i];

    // 4.1 validation TLB
    int frame = findFrameTLB(TLB, page);
    if (frame != -1)
    {
      TLBCount++;
    }

    // 4.2 Memoire Physique
    if (frame == -1)
    {
      if(tablePage[page][1] != 1)
      {
        // Page fault
        std::cout << "Page fault: " << page << " non-chargée dans la table" << std::endl;
        pageFaultCount++;

        // Charger la page
        int physical_addresse = chargerPageDansFrame(page, indexFrame, memPhysique);
        tablePage[page][1] = 1;
        tablePage[page][0] = physical_addresse;
        indexFrame++;
      }

      // Deja en memoire, recupere le frame et met a jour le TLB
      frame = tablePage[page][0];
      addToTLB(TLB, page, frame);
    }

    adressePhysique[i] = frame + offset;
	}

	// 5. Ecrire le fichier de sortie
  int total_adresse = bits_page.size();
  affichageResultats(adresseLogique, adressePhysique, memPhysique, pageFaultCount, TLBCount, total_adresse);

	return 0;
}

