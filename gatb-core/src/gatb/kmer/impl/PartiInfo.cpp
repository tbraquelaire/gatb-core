/*****************************************************************************
 *   GATB : Genome Assembly Tool Box
 *   Copyright (C) 2014  INRIA
 *   Authors: R.Chikhi, G.Rizk, E.Drezen
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include <gatb/kmer/impl/PartiInfo.hpp>
#include <algorithm>

// We use the required packages
using namespace std;

#define DEBUG(a) // printf a

/********************************************************************************/
namespace gatb  {  namespace core  {   namespace kmer  {   namespace impl {
/********************************************************************************/

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Repartitor::computeDistrib (const PartiInfo<5>& extern_pInfo)
{
    /** We allocate a table whose size is the number of possible minimizers. */
    _repart_table.resize (_nb_minims);

    std::vector<ipair> bin_size_vec;
    std::priority_queue< itriple, std::vector<itriple>,compSpaceTriple > pq;

    //sum total bins size
    u_int64_t sumsizes =0;
    for (int ii=0; ii< _nb_minims; ii++)
    {
        // sumsizes +=   extern_pInfo.getNbSuperKmer_per_minim(ii); // _binsize[ii];
        // bin_size_vec.push_back(ipair( extern_pInfo.getNbSuperKmer_per_minim(ii) ,ii));
        sumsizes +=   extern_pInfo.getNbKxmer_per_minim(ii); // _binsize[ii];
        bin_size_vec.push_back(ipair( extern_pInfo.getNbKxmer_per_minim(ii) ,ii));
    }
    u_int64_t mean_size =  sumsizes /  _nbpart;

    DEBUG (("Repartitor : mean size per parti should be :  %lli  (total %lli )\n",mean_size,sumsizes));

    //init space left
    for (int jj = 0; jj < _nbpart; jj++)  {  pq.push (itriple(jj,0,0));  }

    //sort minim bins per size
    std::sort (bin_size_vec.begin (), bin_size_vec.end (), comp_bins);

    DEBUG (("Repartitor : 20 largest estimated bin sizes \n"));
    for (size_t ii=0; ii<20 &&  ii< bin_size_vec.size(); ii++ )
    {
        DEBUG (("binsize [%llu] = %llu \n",bin_size_vec[ii].second,bin_size_vec[ii].first));
    }

    //GC suggestion : put the largest in the emptiest (la plus grosse dans la plus grosse)

    itriple smallest_parti;

    int cur_minim = 0;
    while (cur_minim < _nb_minims)
    {
        //get emptiest parti
        smallest_parti = pq.top(); pq.pop();

        //put largest bin in it
        _repart_table[bin_size_vec[cur_minim].second] = smallest_parti.first;

        //update space used in this bin, push it back in the pq
        smallest_parti.second += bin_size_vec[cur_minim].first;
        smallest_parti.third ++;

        if (smallest_parti.third < 1000) // don't put more than 1000 minimizers in a single bin, bcalm won't like it
            pq.push (smallest_parti);


        DEBUG (("Repartitor : affected minim %llu to part %llu  space used %llu  (msize %llu) \n",
            bin_size_vec[cur_minim].second,smallest_parti.first,
            smallest_parti.second , bin_size_vec[cur_minim].first
        ));

        cur_minim++;
    }
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Repartitor::load (tools::storage::impl::Group& group)
{
    tools::storage::impl::Storage::istream is (group, "minimRepart");
    is.read ((char*)&_nbpart,     sizeof(_nbpart));
    is.read ((char*)&_mm,         sizeof(_mm));
    is.read ((char*)&_nb_minims,  sizeof(_nb_minims));

    /** We allocate a table whose size is the number of possible minimizers. */
    _repart_table.resize (_nb_minims);

    is.read ((char*)_repart_table.data(), sizeof(Value) * _nb_minims);
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Repartitor::save (tools::storage::impl::Group& group)
{
    tools::storage::impl::Storage::ostream os (group, "minimRepart");
    os.write ((const char*)&_nbpart,                sizeof(_nbpart));
    os.write ((const char*)&_mm,                    sizeof(_mm));
    os.write ((const char*)&_nb_minims,             sizeof(_nb_minims));
    os.write ((const char*)_repart_table.data(),    sizeof(Value) * _nb_minims);
    os.flush();
}

/*********************************************************************
** METHOD  :
** PURPOSE :
** INPUT   :
** OUTPUT  :
** RETURN  :
** REMARKS :
*********************************************************************/
void Repartitor::printInfo ()
{
    size_t nbMinimizers = 1 << (_mm*2);
    printf("Repartitor : nbMinimizers=%d\n", nbMinimizers);
    for(int ii=0; ii<nbMinimizers; ii++ )  {  printf("   table[%i] = %i \n",ii,_repart_table[ii]); }
}

/********************************************************************************/
} } } } /* end of namespaces. */
/********************************************************************************/
