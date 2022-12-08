//*******************************************************************
//*																	*
//* Programme: GenerePhoton.cpp	       							    *
//*																	*
//*******************************************************************
//*																	*
//* Description:													*
//*		Génération de photons.                     					*
//*																	*
//* nom1 :                                                          *
//* cip1:                                                           *
//*																	*
//* nom2 :                                                          *
//* cip2:                                                           *
//*******************************************************************


#include "GenerePhoton.h"
#include "inter.h"
#include "util.h"
#include "utilalg.h"
#include "aff.h"
#include "segment.h"
#include "ensemble.h"
#include "geo.h"
#include "mat.h"
#include <math.h>
#include "Point.h"
#include "time.h"
#include "main.h"
#include <stdio.h>
#include "spotlight.h"
#include "ponctuelle.h"
#include "settingTracePhoton.h"

reel CalculerIntensite(Couleur);
reel CalculerIntensite(Couleur coul)
{
    return ((coul.rouge() + coul.bleu() + coul.vert())/ 3.0);
}
 
booleen GenerePhotons( const Camera& camera, Objet* scene )
{


    const Lumiere* lum = NULL;

    PhotonMap* CaustiqueMap = pFenAff3D->PhotonTracing()->PhotonMapCaustique();

//  int nbLum = camera.NbLumiere(); // pour avoir le nombre de lumiere
    


    printf( "\nGeneration des photons...\n");

    clock_t  clk = clock();

//---------------------------------------------------... à compléter
    point pt_origin;
    vecteur directionRayon;
    reel distanceInter;
    vecteur vn;
    Couleurs couleursInter;
    Couleur tempResult;
    reel intensiteTotal = 0.0;
    reel distTotal = 0.0;
    int it = 0;

    //Pour calculer la proportion des photons selon l'intansité des lum.
    for (int l = 0; l < camera.NbLumiere(); l++)
    {
        intensiteTotal += CalculerIntensite(camera.GetLumiere(l)->Intensite());
    }
    

    for (int l = 0; l < camera.NbLumiere(); l++)
    {
        lum = camera.GetLumiere(l);

        //Le ratio de la lumière actuel
        reel ratio = CalculerIntensite(camera.GetLumiere(l)->Intensite())/ intensiteTotal;

        for (int i = 0; i < int(NB_PHOTON_CAUSTIQUE * ratio); i++)
        {
            distTotal = 0;
            directionRayon = lum->RayonAleatoire();
            pt_origin = lum->Position();
            

            if (Objet_Inter(*scene, pt_origin, directionRayon, &distanceInter, &vn, &couleursInter) && couleursInter.reflechi() != Couleur(0, 0, 0))
            {
                while (couleursInter.reflechi() != Couleur(0, 0, 0))
                { 
                    tempResult = NOIR;
                    distTotal += pow(distanceInter, 2);
                   // tempResult = (lum->Intensite()/ distTotal) * couleursInter.reflechi() ;
                    tempResult = (lum->Intensite() * couleursInter.reflechi() *.5)/ distTotal;
                    pt_origin = (directionRayon * distanceInter) + pt_origin;
                    directionRayon = 2 * ((vn * (-directionRayon)) * vn) - (-directionRayon);
                    directionRayon.normalise();

                    Objet_Inter(*scene, pt_origin, directionRayon, &distanceInter, &vn, &couleursInter);
                }
                reel pdm = CalculerIntensite(couleursInter.diffus()) / 3;
                reel rdm = (rand() % 100) / 100.0;            
                if (rdm > pdm)
                {
                    pFenAff3D->PhotonTracing()->PhotonMapCaustique()->Store(tempResult, (directionRayon * distanceInter) + pt_origin, directionRayon);
                }
                
            }
        }
    }



//------------------------------------------------------------------


    printf( "\nFin du trace de photon\n" );

    printf("Balancement du photon map caustique...\n");
    CaustiqueMap->Balance();
    printf("Fin du balancement du photon map caustique\n\n");

    clk = clock() - clk;
    clk /= CLOCKS_PER_SEC;

    Affiche( *CaustiqueMap, &scene, camera );


    printf( "Temps pour generer les photons : %d:%02d\n\n", clk / 60, clk % 60 );

    return VRAI;
}



void Affiche( const PhotonMap& table, Objet **scene, const Camera& camera )
{
    Attributs* a = new Attributs;

    Couleur cd( 1,1,1 );

    a->diffus( cd );
    a->ambiant( cd );

    Ensemble* ens = new Ensemble;
    const Lumiere* lum = NULL;

    for ( int i = 1; i <= table.NbPhotons(); i += 100 )
    {
        point p1;
        p1 = table[i].position();


        vecteur vec;
        vec = table[i].PhotonDir();

        vec = -vec;

        point p2 = p1 + ( vec );

        Segment* seg = new Segment( p1, p2 );
        seg->attributs( a );
        ens->ajoute( seg );
    }

    // affiche la direction du spot light
    for ( int j = 0; j < camera.NbLumiere(); j++ )
    {
        lum = camera.GetLumiere(j);

        if ( lum->Type() == unSpotlight )
        {
            Spotlight * spot = (Spotlight*)lum;
            // direction du spot light
            point p2 = spot->Position() + ( spot->GetDirection() * 2 );
            Segment* seg = new Segment( spot->Position(), p2 );
    
            seg->attributs( a );
            ens->ajoute( seg );
        }
    }

    ens->attributs( a );

    ((Ensemble*)(*scene))->ajoute( ens );
}