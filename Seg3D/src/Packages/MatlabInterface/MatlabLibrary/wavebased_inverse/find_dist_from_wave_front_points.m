function dist=find_dist_from_wave_front_points(interp_curve_points,pts)
% dist=find_dist_from_wave_front_points(curve_points,pts)

N=size(interp_curve_points,1);
M=size(pts,1);
if N==0
    dist=ones(1,M)*100;
    return;
end


heart_center=mean(pts);
heart_center=[20 20 0]; % changed for epi_endo heart
%
%  FIX!!! for different heart geometries
%
 target_pts = size(pts,1);
% v1=pts-ones(target_pts,1)*heart_center;
% v2=interp_curve_points-ones(N,1)*heart_center;

for i=1:target_pts
%     nv2=sqrt(sum(v2'.*v2'));
%     nv1=norm(v1(i,:));
%     ang=acos( (v1(i,:)*v2')./(nv1*nv2));
%     rad=.5*(nv2+nv1*ones(size(nv2)));
%     d=ang.*rad;
    temp=interp_curve_points-ones(N,1)*pts(i,:);
    d=sqrt(sum(temp'.*temp'));
        
    [dist(i), ind]=min(d);
end