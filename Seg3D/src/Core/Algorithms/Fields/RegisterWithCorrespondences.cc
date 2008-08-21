#include <Core/Algorithms/Fields/RegisterWithCorrespondences.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
RegisterWithCorrespondencesAlgo::runM(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output)
{
  algo_start("RegisterWithCorrespondences",false);
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  if (Cors1.get_rep() == 0)
  {
    error("No Correspondence1 input field");
    algo_end(); return (false);
  }
  if (Cors2.get_rep() == 0)
  {
    error("No Correspndence2 input field");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input);
  //output = CreateField(fi,input->mesh());
  //output = CreateField(fi);
  output = input;
  output.detach();
  output->mesh_detach();
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);      
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh* icors1 = Cors1->vmesh();
  VMesh* icors2 = Cors2->vmesh();
  
  //get the number of nodes in input field  
  //VMesh::size_type num_nodes = imesh->num_nodes();
  
  VMesh::Node::size_type num_cors1, num_cors2, num_pts; 
  icors1->size(num_cors1);
  icors2->size(num_cors2);
  imesh->size(num_pts);
  
  vector<double> coefs;//(3*num_cors1+9);
  vector<double> rside;//(3*num_cors1+9);

  if (num_cors1 != num_cors2)
  {
    error("Number of correspondence points does not match");
    algo_end(); return (false);
  }
  
  // Request that it generates the node matrix
  imesh->synchronize(SCIRun::Mesh::NODES_E);
  
  //create B for big matrix//
  MatrixHandle BMat;
  BMat=new DenseMatrix(num_cors1+4,num_cors1+4);
  DenseMatrix *Bm = dynamic_cast<DenseMatrix*>(BMat->dense());
  VMesh::Node::iterator it;
  SCIRun::Point P;

  
  for(int L1=0;L1<num_cors1;++L1)
  {
    it=L1;
    icors2->get_point(P,*(it));
    //horizontal x,y,z
    Bm->put(0,L1,P.x());
    Bm->put(1,L1,P.y());
    Bm->put(2,L1,P.z());
    Bm->put(3,L1,1);
    
    //vertical x,y,z
    Bm->put(L1+4,num_cors1,P.x());
    Bm->put(L1+4,num_cors1+1,P.y());
    Bm->put(L1+4,num_cors1+2,P.z());
    Bm->put(L1+4,num_cors1+3,1);
  }
   for(int L1=num_cors1;L1<num_cors1+4;++L1)
  {
    Bm->put(0,L1,0);
    Bm->put(1,L1,0);
    Bm->put(2,L1,0);
    Bm->put(3,L1,0);
  }
  
  //put in sigmas
  MatrixHandle SMat;
  radial_basis_func(icors2, icors2, SMat);
  SMat->get_data_pointer();
  double temp = 0; 
  
  for(int i=0;i<num_cors1;++i)
  {
    for(int j=0;j<num_cors1;++j)
    {
      temp=SMat->get(i,j);
      Bm->put(4+i,j,temp);    }
  }
  
  //Create big matrix //
  MatrixHandle BigMat;
  BigMat=new DenseMatrix(3*num_cors1+12,3*num_cors1+12);
  DenseMatrix *Bigm = dynamic_cast<DenseMatrix*>(BigMat->dense());
  
  for(int i=0;i<(3*num_cors1+12);++i)
  {
    for(int j=0;j<(3*num_cors1+12);++j)
    {
      Bigm->put(i,j,0);
    }
  }
  for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(i,j,temp);
    }
  }
  for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(num_cors1+4+i,num_cors1+4+j,temp);
       
    }
  }
   for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(2*num_cors1+8+i,2*num_cors1+8+j,temp);
    }
  }
  
  //create right side of equation//
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.x());
  }
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.y());
  }
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.z());
  }
  
  //Solve system of equations//
  Bigm->solve(rside,coefs,0);
  

  make_new_points(imesh, icors2, coefs, *omesh);
  
  algo_end(); return (true);
}


bool
RegisterWithCorrespondencesAlgo::runA(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output)
{
  algo_start("RegisterWithCorrespondences",false);
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  if (Cors1.get_rep() == 0)
  {
    error("No Correspondence1 input field");
    algo_end(); return (false);
  }
  if (Cors2.get_rep() == 0)
  {
    error("No Correspndence2 input field");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input);
  //output = CreateField(fi,input->mesh());
  output = input;
  output.detach();
  output->mesh_detach();
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);      
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh* icors1 = Cors1->vmesh();
  VMesh* icors2 = Cors2->vmesh();
  
  //get the number of nodes in input field  
  //VMesh::size_type num_nodes = imesh->num_nodes();
  
  VMesh::Node::size_type num_cors1, num_cors2, num_pts; 
  icors1->size(num_cors1);
  icors2->size(num_cors2);
  imesh->size(num_pts);
  
  vector<double> coefs;//(3*num_cors1+9);
  vector<double> rside;//(3*num_cors1+9);

  if (num_cors1 != num_cors2)
  {
    error("Number of correspondence points does not match");
    algo_end(); return (false);
  }
  
  // Request that it generates the node matrix
  imesh->synchronize(SCIRun::Mesh::NODES_E);
  
  //create B for big matrix//
  MatrixHandle BMat;
  BMat=new DenseMatrix(num_cors1+4,num_cors1+4);
  DenseMatrix *Bm = dynamic_cast<DenseMatrix*>(BMat->dense());
  VMesh::Node::iterator it;
  SCIRun::Point P;

  
  for(int L1=0;L1<num_cors1;++L1)
  {
    it=L1;
    icors2->get_point(P,*(it));
    double tep=P.x();
    

    //horizontal x,y,z
    Bm->put(0,L1,P.x());
    Bm->put(1,L1,P.y());
    Bm->put(2,L1,P.z());
    Bm->put(3,L1,1);
    
    //vertical x,y,z
    Bm->put(L1+4,num_cors1,P.x());
    Bm->put(L1+4,num_cors1+1,P.y());
    Bm->put(L1+4,num_cors1+2,P.z());
    Bm->put(L1+4,num_cors1+3,1);
  }
   for(int L1=num_cors1;L1<(num_cors1+4);++L1)
  {
    Bm->put(0,L1,0);
    Bm->put(1,L1,0);
    Bm->put(2,L1,0);
    Bm->put(3,L1,0);
  }
  
  //put in sigmas
  MatrixHandle SMat;
  radial_basis_func(icors2, icors2, SMat);
  SMat->get_data_pointer();
  double temp = 0; 
  
  for(int i=0;i<num_cors1;++i)
  {
    for(int j=0;j<num_cors1;++j)
    {
      temp=SMat->get(i,j);
      Bm->put(4+i,j,temp);    }
  }
  
  //Create big matrix //
  MatrixHandle BigMat;
  BigMat=new DenseMatrix(3*num_cors1+12,3*num_cors1+12);
  DenseMatrix *Bigm = dynamic_cast<DenseMatrix*>(BigMat->dense());
  
  for(int i=0;i<(3*num_cors1+12);++i)
  {
    for(int j=0;j<(3*num_cors1+12);++j)
    {
      Bigm->put(i,j,0);
    }
  }
  for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(i,j,temp);
    }
  }
  for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(num_cors1+4+i,num_cors1+4+j,temp);
       
    }
  }
   for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
       temp=Bm->get(i,j);
       Bigm->put(2*num_cors1+8+i,2*num_cors1+8+j,temp);
    }
  }

  //create right side of equation//
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.x());
  }
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.y());
  }
  for(int i=0;i<4;++i){rside.push_back(0);}
  for(int i=0;i<num_cors1;++i)
  {
    it=i;
    icors1->get_point(P,*(it));
    rside.push_back(P.z());
  }
  
    for(int i=0;i<(num_cors1+4);++i)
  {
    for(int j=0;j<(num_cors1+4);++j)
    {
        temp=Bm->get(i,j);
        printf("%lf :",temp);
    }
    cout<<endl;
  }
  
 
  
  //Solve system of equations//
  Bigm->solve(rside,coefs,0);
  

  make_new_pointsA(imesh, icors2, coefs, *omesh);
  
  algo_end(); return (true);
}


bool
RegisterWithCorrespondencesAlgo::runN(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output)
{
  algo_start("RegisterWithCorrespondences",false);
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  if (Cors1.get_rep() == 0)
  {
    error("No Correspondence1 input field");
    algo_end(); return (false);
  }
  if (Cors2.get_rep() == 0)
  {
    error("No Correspndence2 input field");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input);
  //output = CreateField(fi,input->mesh());
  output = input;
  output.detach();
  output->mesh_detach();
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);      
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
 
  omesh=imesh;
  
  algo_end(); return (true);
}


bool
RegisterWithCorrespondencesAlgo::radial_basis_func(VMesh* Cors, VMesh* points, MatrixHandle& Smat)
{
    VMesh::Node::size_type num_cors, num_pts;
    VMesh::Node::iterator iti,itj;
    SCIRun::Point Pc,Pp;
  
    
    Cors->size(num_cors);
    points->size(num_pts);
    
    double xcomp=0.0,ycomp=0.0,zcomp=0.0,mag=0.0;
    //MatrixHandle SMat;
    Smat=new DenseMatrix(num_pts,num_cors);
    DenseMatrix *Sigma = dynamic_cast<DenseMatrix*>(Smat->dense());

    for(int i=0;i<num_pts;++i)
    {
      for(int j=0;j<num_cors;++j)
      {
        iti=i;
        itj=j;
        
        Cors->get_point(Pc,*(itj));
        points->get_point(Pp,*(iti));
        
        xcomp=Pc.x()-Pp.x();
        ycomp=Pc.y()-Pp.y();
        zcomp=Pc.z()-Pp.z();
        mag=sqrt(pow(xcomp,2.0)+pow(ycomp,2.0)+pow(zcomp,2.0));
        if(mag==0)
        {
          Sigma->put(i,j,0);
        }
        else
        {
          
          double temp=pow(mag,2.0)*log(mag);
          //printf("%lf :",mag);

          Sigma->put(i,j,temp);
          }    
      }
      //cout<<endl;
     }
    return true;
}

bool
RegisterWithCorrespondencesAlgo::make_new_points(VMesh* points, VMesh* Cors, vector<double>& coefs, VMesh& omesh)
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it,itp;
  SCIRun::Point P,Pp;
    
  Cors->size(num_cors);
  points->size(num_pts);
  
  
  int sz=0;
  double sumx=0,sumy=0,sumz=0;
  double sigma=0.0;
  MatrixHandle SMat;
  radial_basis_func(Cors, points, SMat);
  SMat->get_data_pointer();
  
  for(int i=0; i< num_pts;++i)
  {
    sumx=0;sumy=0;sumz=0;
    for(int j=0; j<num_cors; ++j)
    {
      sigma=SMat->get(i,j);
        sumx+=coefs[j]*sigma;
        sumy+=coefs[j+4+num_cors]*sigma;
        sumz+=coefs[j+8+2*num_cors]*sigma;
    }
  
  itp=i;
  points->get_point(Pp,*(itp));
  sz=(int)num_cors;
  
 
  P.x( sumx + Pp.x() * (coefs[sz]) + Pp.y() * (coefs[sz+1]) + Pp.z() * (coefs[sz+2]) + coefs[sz+3]);
  P.y( sumy+Pp.x() * coefs[2*sz+4]+Pp.y()*coefs[2*sz+5]+Pp.z()*coefs[2*sz+6] + coefs[2*sz+7]);
  P.z( sumz+Pp.x() * coefs[3*sz+8]+Pp.y()*coefs[3*sz+9]+Pp.z()*coefs[3*sz+10] + coefs[3*sz+11]);
 

 // double t1=P.x();
//  double t2=P.y();
//  double t3=P.z();
//  cout<<t1<<"\t"<<t2<<"\t"<< t3<<endl;
   
  omesh.set_point(P,*(itp));
  }
  return true;
}

bool
RegisterWithCorrespondencesAlgo::make_new_pointsA(VMesh* points, VMesh* Cors, vector<double>& coefs, VMesh& omesh)
{
  VMesh::Node::size_type num_cors, num_pts;
  VMesh::Node::iterator it,itp;
  SCIRun::Point P,Pp;
    
  Cors->size(num_cors);
  points->size(num_pts);
  for(int i=0; i< num_pts;++i)
  {
    int sz=0;
    itp=i;
    points->get_point(Pp,*(itp));
    sz=(int)num_cors;
  
    P.x( Pp.x() * (coefs[sz]) + Pp.y() * (coefs[sz+1]) + Pp.z() * (coefs[sz+2]) + coefs[sz+3]);
    P.y( Pp.x() * coefs[2*sz+4]+Pp.y()*coefs[2*sz+5]+Pp.z()*coefs[2*sz+6] + coefs[2*sz+7]);
    P.z( Pp.x() * coefs[3*sz+8]+Pp.y()*coefs[3*sz+9]+Pp.z()*coefs[3*sz+10] + coefs[3*sz+11]);

//    P.x( Pp.x() + coefs[sz+3]);
//    P.y( Pp.y()  + coefs[2*sz+7]);
//    P.z( Pp.z()  + coefs[3*sz+11]);
 
    double t1=P.x();
    double t2=P.y();
    double t3=P.z();
    cout<<t1<<"\t"<<t2<<"\t"<< t3<<endl;
   
    omesh.set_point(P,*(itp));
  }
  return true;
}

} // end namespace SCIRun
