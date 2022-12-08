/******************************************************************/
/*                                                                */   
/* Paul Duchesneau 20 160 889                                     */   
/* Tientcheu Tchako David Jeeson 21 151 003                       */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/*                                                                */   
/******************************************************************/






#include "rayons.h"
#include "couleurs.h"
#include "attr.h"
#include "ensemble.h"
#include "point.h"
#include "transfo.h"
#include "inter.h"
#include "vision.h"
#include "util.h"
#include "utilalg.h"
#include <stdio.h>
#include <math.h>



Couleur calculer_intensite_rayon(point O, vecteur dir, Objet* scene, const Camera& camera, entier* it);
Couleur calcul_intensite_pt_inter(Objet* scene, const Camera& camera, vecteur dir, point pt_inter, vecteur vn, Couleurs couleurs, entier *it);


void Enregistre_pixel (int no_x, int no_y, Couleur intens, Fichier f)
// enregistre la couleur du pixel dans le fichier f
// Chacune des composantes de la couleur doit etre de 0 a 1,
// sinon elle sera ramene a une borne.
{
     
	reel r,v,b;
	char c;
 
	intens=intens*255.0;
	r=intens.rouge();
	if (r<0) r=0;
	if (r>255)r=255;
	c=(unsigned char) r;
	f.Wcarac(c);

	v=intens.vert();
	if (v<0) v=0;
	if (v>255) 
		v=255;
	c=(unsigned char) v;
	f.Wcarac(c);

	b=intens.bleu();
	if (b<0) b=0;
	if (b>255) b=255;
	c=(unsigned char) b;
	f.Wcarac(c);

}


booleen TraceRayons(const Camera& camera, Objet *scene, const entier& res, char fichier[])
// Genere un rayon pour chacun des pixel et calcul l'intensite de chacun
{ 
	Couleur Intensite (0.0,0.0,0.0);

	entier nb_pixel_x = res;
	entier nb_pixel_y = res;

	Transformation transfInv = Vision_Normee(camera).inverse(); // transformation de vision inverse

// ...

	//Ouverture du fichier pour y enregistrer le trace de rayon
	Fichier f;
	if ( !f.Open(fichier, "wb") ) return FAUX;

	f.Wchaine("P6\r");
	f.Wentier(res); f.Wcarac(' '); f.Wentier(res);	f.Wchaine("\r");
	f.Wentier(255);	f.Wchaine("\r");
	point pixelPos;
	vecteur dirPixel;
	reel dx = 2.0 / nb_pixel_x;
	reel dy = 2.0 / nb_pixel_y;
	entier maxReflection = 0;


	printf("\nDebut du trace de rayons\n");
	printf ("%4d source(s) lumineuse(s)\n", camera.NbLumiere());
// ...

	for (int no_y = 1; no_y <= nb_pixel_y; no_y++)   
	{ 
		for (int no_x = 1; no_x <= nb_pixel_x; no_x++)    
		{
// ...
			Intensite = Intensite * 0;
			maxReflection = 10;
			pixelPos = point(1.0 - (dx / 2) - (no_x - 1) * dx, 1.0 - (dy / 2) - (no_y - 1) * dy, 1);
			pixelPos = transfInv.transforme(pixelPos);
			dirPixel = vecteur(camera.PO(), pixelPos);
			dirPixel.normalise();

			Intensite = calculer_intensite_rayon(camera.PO(), dirPixel, scene, camera, &maxReflection);


			Enregistre_pixel(no_x, no_y,Intensite, f);
			
		}

		//Imprime le # de ligne rendu
		if ( no_y % 15 == 0 ) printf("\n");
		printf ("%3d ", no_y);	
	}
	printf ("\n\nFin du trace de rayons.\n");


	f.Close();
	return VRAI;


}

Couleur calculer_intensite_rayon(point O, vecteur dir, Objet* scene, const Camera& camera, entier *it)
{
	Couleur result(0, 0, 0);

		reel distance = 0;
		vecteur vNormal = vecteur();
		Couleurs materiel = Couleurs();
		if (Objet_Inter(*scene, O, dir, &distance, &vNormal, &materiel))
		{
			point inter = (dir * distance) + camera.PO();
			result = calcul_intensite_pt_inter(scene, camera, dir, inter, vNormal, materiel, it);
		}
		return result;
}

Couleur calcul_intensite_pt_inter(Objet* scene, const Camera& camera, vecteur dir, point pt_inter, vecteur vn, Couleurs couleurs, entier *it)
{
	Couleur result(0.0, 0.0, 0.0), im(0, 0, 0);

	vecteur L, RO;
	vecteur O(-dir);
	O.normalise();

	vecteur H;
	reel cosA = 0;
	reel cosT = 0;

	booleen ilumine = false;
	reel distanceInter = 0;
	vecteur vecteurtemp = vecteur();
	Couleurs couleurstemp = Couleurs();
	vecteur directionLumiere = vecteur();
	reel distanceLumiere = 0;



	if (((O * vn) / O.norme()) < 0)
	{
		vn = -vn;
	}


	for (int i = 0; i < camera.NbLumiere(); i++)
	{
		result = Couleur(0, 0, 0);
		const Lumiere* lum = camera.GetLumiere(i);
		directionLumiere = vecteur(pt_inter, camera.Position(i));
		distanceLumiere = directionLumiere.norme();
		directionLumiere.normalise();

		/*if (Objet_Inter(*scene, pt_inter, directionLumiere, &distanceInter, &vecteurtemp, &couleurstemp))
		{
			ilumine = abs(distanceInter) > abs(distanceLumiere) ? VRAI : FAUX;
		}
		else
		{
			ilumine = VRAI;
		}*/
		ilumine = lum->Eclaire(pt_inter) ? VRAI : FAUX;

		if (ilumine)
		{
			L = vecteur(pt_inter, camera.Position(i));
			L.normalise();

			cosT = L * vn;
			cosT = cosT < 0 ? 0 : cosT;

			//Difus		
			//result = result + (camera.Diffuse(i) * couleurs.diffus()) * cosT;
			result = result + (lum->Intensite() * couleurs.diffus()) * cosT;

			//Ambiant
			//result = result + (couleurs.diffus() * camera.Ambiante(i));
			result = result + (lum->IntensiteAmbiante() * couleurs.diffus());

			//speculaire
			H = (L + O);
			H.normalise();

			cosA = H * vn;
			cosA = cosA < 0 ? 0 : cosA;
			cosA = cosT <= 0 ? 0 : cosA;
			//result = result + (couleurs.speculaire() * camera.Diffuse(i)) * pow(cosA, 90);
			result = result + (couleurs.speculaire() * lum->Intensite()) * pow(cosA, 90);
		}
		else
		{
			//result = result + (couleurs.diffus() * camera.Ambiante(i));
			result = result + (couleurs.diffus() * lum->Intensite());
		}


	}

	if (couleurs.reflechi() != Couleur(0, 0, 0) && *it > 0)
	{
		RO = 2 * ((vn * O) * vn) - O;
		RO.normalise();
		*it = *it - 1;
		im = calculer_intensite_rayon(pt_inter, RO, scene, camera, it);
	}

	return result + (couleurs.reflechi() * im);
	


}
  
