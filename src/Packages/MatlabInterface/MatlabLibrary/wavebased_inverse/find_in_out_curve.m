function in_curve_flag=find_in_out_curve(curve_points,pts)
%in_curve_flag=find_in_out_curve(curve_points,pts)

in_curve_flag=ones(size(pts,1),1);
N=size(curve_points,1);
curve_points(N+1,:)=curve_points(1,:);
N=N+1;

zp=-40;  % projection plane
z0=60; % view point

curve_points_p=project_point_wise(curve_points,zp,z0);
pts_p=project_point_wise(pts,zp,z0);


for i=1:size(pts,1)
    v1= (curve_points_p-ones(N,1)*pts_p(i,:));
    nv1=sqrt(sum(v1'.*v1'));
    cos_ang=sum(v1(1:N-1,:)'.*v1(2:N,:)')./(nv1(1:N-1).*nv1(2:N));
    temp=cross(v1(1:N-1,:),v1(2:N,:));
    ang_sign=sign(temp(:,3)');
    ang(i)=sum(abs(acos(cos_ang)).*ang_sign);
end
in_curve_flag(find(abs(ang)>pi))=-1;
