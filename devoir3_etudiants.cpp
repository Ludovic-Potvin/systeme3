////////////////////////////////////////////////////////////////
//devoir3.cpp
//Créé par: Sara Séguin
//Date: 5 novembre 2025
//CE FICHIER N'EST PAS EXHAUSTIF, IL DONNE SIMPLEMENT DES EXEMPLES ET UNE CERTAINE STRUCTURE.À Vous de modifier.
////////////////////////////////////////////////////////////////

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
    std::cout << "Page fault %: " << ( (double)pageFaultCount / totalAdresses ) * 100 << "\n";
}

int chargerPageDansFrame(int page, int frame, char memPhysique[TAILLE_MEMOIRE_PHYSIQUE]){
    std::ifstream fichier("simuleDisque.bin", std::ios::binary);
    int physical_addresse = frame * PAGE_t;

    fichier.seekg(page * PAGE_t);
    fichier.read(&memPhysique[physical_addresse], PAGE_t);
    fichier.close();

    return physical_addresse;
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
  int TLB[16] = {};
	
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

	for(int i=0;i<bits_page.size();i++)
	{
    int page = bits_page[i];
    int offset = bits_offset[i];
    int frame = -1;

    // TODO Validate TLB

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

      frame = tablePage[page][0];
    }

    adressePhysique[i] = frame + offset;
	}

	// 5. Ecrire le fichier de sortie
  int total_adresse = bits_page.size();
  affichageResultats(adresseLogique, adressePhysique, memPhysique, pageFaultCount, total_adresse);

	return 0;
}

