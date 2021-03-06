function [interp_sampled_curve_points,sampled_curve_points,curve_len]=resample_curve(curve_points)

%[interp_sampled_curve_points,sampled_curve_points,curve_len]=resample_curve(curve_points)
% This function resamples the curve by moving a pointer on the curve points
% and pick a point after reaching a certain length. This function does not
% resample curves with less than 10 points. The distance between points
% will be 5mm if the curve has less than 20 points and 10mm if curve has more than 20 points

N=size(curve_points,1);

% ANDREW ADDITION FOR DIFFERENT GEOMETRIES
%ratio of the size of the heart geometries
%set to 1 for the utah heart
heart_ratio = 611.477;

if size(curve_points,1)<10
    sampled_curve_points=curve_points;
    curve_len=0;% Not being used
else
    curve_len_step=10/heart_ratio;
    if size(curve_points,1)<20
        curve_len_step=5/heart_ratio;
    end

    sampled_curve_points(1,:)=curve_points(1,:);
    
    curve_points_num=1;
    curve_len=0;
    for i=2:N
        temp=floor(curve_len/curve_len_step);
        curve_len=curve_len+norm(curve_points(i,:)-curve_points(i-1,:));
        if floor(curve_len/curve_len_step)>temp
            curve_points_num=curve_points_num+1;
            sampled_curve_points(curve_points_num,:)=curve_points(i,:);
        end
    end
end
N=size(sampled_curve_points,1);

%smooth the curve for test
M=N;


X=sampled_curve_points(:,1);
Y=sampled_curve_points(:,2);
Z=sampled_curve_points(:,3);

sp1=spap2([-4/M:1/M:1+4/M],4,[-2/N:1/N:1+2/N],[X(N-1) X(N) X' X(1) X(2) X(3)]);
sp2=spap2([-4/M:1/M:1+4/M],4,[-2/N:1/N:1+2/N],[Y(N-1) Y(N) Y' Y(1) Y(2) Y(3)]);
sp3=spap2([-4/M:1/M:1+4/M],4,[-2/N:1/N:1+2/N],[Z(N-1) Z(N) Z' Z(1) Z(2) Z(3)]);

X_s=fnval(sp1,0:1/90:1);
Y_s=fnval(sp2,0:1/90:1);
Z_s=fnval(sp3,0:1/90:1);

interp_sampled_curve_points=[X_s' Y_s' Z_s'];
