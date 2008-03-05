function new_pts=project_point_wise(pts,zp,z0)
%zp=projection plane location
%z0=view point location
temp(:,1)=pts(:,1)./(pts(:,3)-z0)*(zp-z0);
temp(:,2)=pts(:,2)./(pts(:,3)-z0)*(zp-z0);
temp(:,3)=zeros(size(pts,1),1);
new_pts=temp;
 
    