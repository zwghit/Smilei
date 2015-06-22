// CLOBAL COORDINATES: 
//                             MPI_minGlobal                                                                        MPI_maxGlobal
//                      --------<===================================== gs ===================================>------------
//     GLOBAL INDICES:          0                                  .                                        nspace_global
//                           ix+oversize                                                                  ix+oversize
//                      ------------------------------------       .              ------------------------------------
//                      |   |   |     ...          |   |   |       .              |   |   |   |   ...    |   |   |   |
//                      |   |   |     ...          |   |   |       .              |   |   |   |   ...    |   |   |   |
//                      ------------------------------------       .              ------------------------------------
//                            MPI_minLocal      MPI_maxLocal       .               MPI_minLocal          MPI_maxLocal
//                                                 ----------------------------------------                 
//                                                 |   |   |       .              |   |   |
//                                                 |   |   |       .              |   |   |
//                                                 ----------------------------------------
// LOCAL COORDINATES:                             x(0) rlb        x(ix)             rub  x(nspace)
//                                                 ----<============= length =========>----
//     LOCAL INDICES:                              0   lb                            ub   nspace

#include "SmileiMPI.h"

#include <cmath>
#include <cstring>

#include <iostream>
#include <sstream>

#include "PicParams.h"
#include "DiagParams.h"
#include "Diagnostic.h"
#include "Tools.h"

#include "ElectroMagn.h"
#include "Field.h"

#include "Species.h"
#include "Hilbert_functions.h"

using namespace std;

SmileiMPI::SmileiMPI( int* argc, char*** argv )
{    
    int mpi_provided;

    MPI_Init_thread( argc, argv, MPI_THREAD_FUNNELED, &mpi_provided );
    if (mpi_provided == MPI_THREAD_SINGLE){
        MESSAGE("openMP not supported");
    }

    SMILEI_COMM_WORLD = MPI_COMM_WORLD;
    MPI_Comm_size( SMILEI_COMM_WORLD, &smilei_sz );
    MPI_Comm_rank( SMILEI_COMM_WORLD, &smilei_rk );

}

SmileiMPI::SmileiMPI( SmileiMPI *smpi )
{
    SMILEI_COMM_WORLD = smpi->SMILEI_COMM_WORLD;
    MPI_Comm_size( SMILEI_COMM_WORLD, &smilei_sz );
    MPI_Comm_rank( SMILEI_COMM_WORLD, &smilei_rk );

    oversize = smpi->oversize;
    cell_starting_global_index = smpi->cell_starting_global_index;
    min_local = smpi->min_local;
    max_local = smpi->max_local;

    n_space_global = smpi->n_space_global;
    patch_count.resize(smilei_sz, 0);

}

SmileiMPI::~SmileiMPI()
{
    int status = 0;
    MPI_Finalized( &status );
    if (!status) MPI_Finalize();

}

void SmileiMPI::bcast( InputData& input_data )
{
    DEBUG(10,"broadcast namelist");
    bcast(input_data.namelist);    

    input_data.parseStream();
    
    // Randomization
    unsigned long seedTime=0;
    input_data.extract("random_seed",seedTime);
    srand(seedTime+getRank());
    
}

void SmileiMPI::bcast( string& val )
{
    int charSize=0;
    if (isMaster()) charSize = val.size()+1;
    MPI_Bcast(&charSize, 1, MPI_INT, 0, SMILEI_COMM_WORLD);

    char tmp[charSize];
    strcpy(tmp, val.c_str());
    MPI_Bcast(&tmp, charSize, MPI_CHAR, 0, SMILEI_COMM_WORLD);

    if (!isMaster()) val=tmp;

}

void SmileiMPI::init( PicParams& params )
{

    unsigned int Npatches, Tload, local_load;

    oversize.resize(params.nDim_field, 0);
    cell_starting_global_index.resize(params.nDim_field, 0);
    min_local.resize(params.nDim_field, 0.);
    max_local.resize(params.nDim_field, 0.);
    n_space_global.resize(params.nDim_field, 0);
    patch_count.resize(smilei_sz, 0);

    interParticles.initialize(0,params.nDim_particle); 
 
    //Compute total Load
    Tload = 0;
    Npatches = params.number_of_patches[0];
    for (unsigned int i = 1; i < params.nDim_field; i++) Npatches *=  params.number_of_patches[i];

    for (unsigned int ipatch = 0; ipatch < Npatches; ipatch++){
        local_load = 1. ;
        Tload += local_load;
    }


}


void SmileiMPI::sumRho( ElectroMagn* EMfields )
{
    sumField( EMfields->rho_ );

}

void SmileiMPI::sumRhoJ( ElectroMagn* EMfields )
{
    // sum total charge density and currents
    //sumField( EMfields->rho_ );
    sumField( EMfields->Jx_ );
    sumField( EMfields->Jy_ );
    sumField( EMfields->Jz_ );
   
}
void SmileiMPI::sumRhoJs( ElectroMagn* EMfields, int ispec, bool currents )
{
   // sum density and currents for all species
   sumField( EMfields->rho_s[ispec] );
   if(currents){
       sumField( EMfields->Jx_s[ispec] );
       sumField( EMfields->Jy_s[ispec] );
       sumField( EMfields->Jz_s[ispec] );
   }
}

void SmileiMPI::exchangeE( ElectroMagn* EMfields )
{
    exchangeField( EMfields->Ex_ );
    exchangeField( EMfields->Ey_ );
    exchangeField( EMfields->Ez_ );

}
void SmileiMPI::exchangeE( ElectroMagn* EMfields, unsigned int clrw )
{
    exchangeField_movewin( EMfields->Ex_, clrw );
    exchangeField_movewin( EMfields->Ey_, clrw );
    exchangeField_movewin( EMfields->Ez_, clrw );

}

void SmileiMPI::exchangeB( ElectroMagn* EMfields )
{
    exchangeField( EMfields->Bx_ );
    exchangeField( EMfields->By_ );
    exchangeField( EMfields->Bz_ );

}
void SmileiMPI::exchangeB( ElectroMagn* EMfields, unsigned int clrw )
{
    exchangeField_movewin( EMfields->Bx_, clrw );
    exchangeField_movewin( EMfields->By_, clrw);
    exchangeField_movewin( EMfields->Bz_, clrw );

}

void SmileiMPI::exchangeBm( ElectroMagn* EMfields )
{
    exchangeField( EMfields->Bx_m );
    exchangeField( EMfields->By_m );
    exchangeField( EMfields->Bz_m );

}
void SmileiMPI::exchangeBm( ElectroMagn* EMfields, unsigned int clrw )
{
    exchangeField_movewin( EMfields->Bx_m, clrw );
    exchangeField_movewin( EMfields->By_m, clrw );
    exchangeField_movewin( EMfields->Bz_m, clrw );

}

void SmileiMPI::exchangeAvg( ElectroMagn* EMfields )
{
    exchangeField( EMfields->Ex_avg );
    exchangeField( EMfields->Ey_avg );
    exchangeField( EMfields->Ez_avg );
    exchangeField( EMfields->Bx_avg );
    exchangeField( EMfields->By_avg );
    exchangeField( EMfields->Bz_avg );
}

// Returns the rank of the MPI process currently owning patch h.
int SmileiMPI::hrank(int h)
{
    if (h == MPI_PROC_NULL) return MPI_PROC_NULL;

    unsigned int patch_counter,rank;
    rank=0;
    patch_counter = patch_count[0];
    while (h >= patch_counter) {
        rank++;
        patch_counter += patch_count[rank];
    }
    return rank;
}

void SmileiMPI::computeGlobalDiags(Diagnostic* diags, int timestep)
{
    if (timestep % diags->scalars.every == 0) computeGlobalDiags(diags->scalars, timestep);
    //computeGlobalDiags(probes);
    //computeGlobalDiags(phases);
}

void SmileiMPI::computeGlobalDiags(DiagnosticScalar& scalars, int timestep)
{
    int nscalars(0);
    for(vector<pair<string,double> >::iterator iter = scalars.out_list.begin(); iter !=scalars.out_list.end(); iter++) {
	if ( ( (iter->first).find("Min") == std::string::npos ) && ( (iter->first).find("Max") == std::string::npos ) ) {
	    MPI_Reduce(isMaster()?MPI_IN_PLACE:&((*iter).second), &((*iter).second), 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else if ( (iter->first).find("MinCell") != std::string::npos ) {
	    vector<pair<string,double> >::iterator iterVal = iter-1;
	    val_index minVal;
	    minVal.val   = (*iterVal).second;
	    minVal.index = (*iter).second;
	    MPI_Reduce(isMaster()?MPI_IN_PLACE:&minVal, &minVal, 1, MPI_DOUBLE_INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
	}
	else if ( (iter->first).find("MaxCell") != std::string::npos ) {
	    vector<pair<string,double> >::iterator iterVal = iter-1;
	    val_index maxVal;
	    maxVal.val   = (*iterVal).second;
	    maxVal.index = (*iter).second;
	    MPI_Reduce(isMaster()?MPI_IN_PLACE:&maxVal, &maxVal, 1, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
	}
    }

    if (isMaster()) {

	double Etot_part = scalars.getScalar("Eparticles");
	double Etot_fields = scalars.getScalar("EFields");
	
	double Energy_time_zero = 0.;//scalars.Energy_time_zero;
	double poyTot = scalars.getScalar("Poynting");

	double Elost_part = scalars.getScalar("Elost");
	double Emw_lost  = scalars.getScalar("Emw_lost");
	double Emw_lost_fields = scalars.getScalar("Emw_lost_fields");

	double Total_Energy=Etot_part+Etot_fields;

	//double Energy_Balance=Total_Energy-(Energy_time_zero+poyTot)+Elost_part+Emw_lost+Emw_lost_fields;
	double Energy_Balance=Total_Energy-(Energy_time_zero);
	double Energy_Bal_norm(0.);
	if (scalars.EnergyUsedForNorm>0.)
	    Energy_Bal_norm=Energy_Balance/scalars.EnergyUsedForNorm;
	scalars.EnergyUsedForNorm = Total_Energy;
	cout << " Energy_Bal_norm =" << Energy_Bal_norm << endl;

	scalars.setScalar("Etot",Total_Energy);
	scalars.setScalar("Ebalance",Energy_Balance);
	scalars.setScalar("Ebal_norm",Energy_Bal_norm);

	if (timestep==0) {
	    scalars.Energy_time_zero  = Total_Energy;
	    scalars.EnergyUsedForNorm = Energy_time_zero;
	}


    }
	

    scalars.write(timestep);

}
