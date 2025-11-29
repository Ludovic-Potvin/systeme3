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
	//Initialisation et déclarations
	char memPhysique[TAILLE_MEMOIRE_PHYSIQUE] = {0}; //Mémoire physique
	int adressePhysique[1000] = {0}; //Adresses Physiques
	int tablePage[256][2]={0}; //Table de page
	std::vector<int>adresseLogique; //Adresses Logiques
  int TLB[16] = {};
	
	//Lire le fichier d'adresses à traduire
  std::ifstream fichier("addresses.txt");
  int valeur;
  while (fichier >> valeur) {
    adresseLogique.push_back(valeur);
  }


	//Traduire l'adresse physique en adresse logique
	//1. Traduire l'entier en bits
	
	//Stocker les nombres binaires dans un vecteur
	std::vector<int>bits_offset,bits_page; //Un vecteur pour les bits de page et un autre pour les bits d'offset
	
	//Crééer un masque pour lire juste les bits 0 à 7 (offset)
	unsigned r = 0;
	r = createMask(0,7);
	
	//Créer un masque pour lire juste les bits 8 à 15 (page)
	unsigned r2 = 0;
	r2 = createMask(8,15);
	
	//Boucler sur les 1000 adresses
	for(int i =0; i< adresseLogique.size() ; i++)
	{
		int A = adresseLogique[i];
		int page = (r2 & A) >> 8;
		int offset = r & A;

		bits_page.push_back(page);
		bits_offset.push_back(offset);
	}

	// Table de pages
	// Une adresse à la fois, vérifier si elle est dans la table de page
  // Charger les adressesPhysique
	
  int indexFrame = 0;
  int pageFaultCount = 0;
	for(int i=0;i<bits_page.size();i++)
	{
    if(tablePage[bits_page[i]][1] != 1)
    {
      std::cout << "Page fault: " << bits_page[i] << " non-chargée dans la table" << std::endl;
      pageFaultCount++;

      //Charger la page
      int physical_addresse = chargerPageDansFrame(bits_page[i], indexFrame, memPhysique);
      tablePage[bits_page[i]][1] = 1;
      tablePage[bits_page[i]][0] = physical_addresse;
      indexFrame++;
    }

    adressePhysique[i] = tablePage[bits_page[i]][0] + bits_offset[i];
	}



	//Ecrire le fichier de sortie
  std::ofstream out("resultats.txt");

  for (int i = 0; i < adresseLogique.size(); i++) {
    int value = (char)memPhysique[adressePhysique[i]];
    out << "Virtual address: " << adresseLogique[i]
      << " Physical address: " << adressePhysique[i]
      << " Value: " << value
      << " Value bin: " << std::bitset<8>(memPhysique[adressePhysique[i]])
      << std::endl;
  }

  // Display stats
  std::cout << "Page fault %: " << ((double)pageFaultCount/bits_page.size())*100 << "\n";

	return 0;
}

