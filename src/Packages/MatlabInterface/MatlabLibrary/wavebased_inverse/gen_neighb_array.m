function neighb_array = gen_neighb_array(pts,fac)

col1 = 1:size(pts,1);

neighb_array = [col1' zeros(size(pts,1),9)];
   
for i=1:size(fac,1)
    x=fac(i,1);
    y=fac(i,2);
    z=fac(i,3);
    
    % neighbors of x
    index = max(find(neighb_array(x,:)));
    y_t = 0;
    z_t = 0;
    for j = 2:index
        if(neighb_array(x,j) == y)
            y_t = 1;
        end
        if(neighb_array(x,j) == z)
            z_t = 1;
        end
    end
    if(y_t == 0)
        neighb_array(x,index+1) = y;
    end
    if(z_t == 0)
        neighb_array(x,index+2-y_t) = z;
    end
    
    %neighbors of y
    index = max(find(neighb_array(y,:)));
    x_t = 0;
    z_t = 0;
    for j = 2:index
        if(neighb_array(y,j) == x)
            x_t = 1;
        end
        if(neighb_array(y,j) == z)
            z_t = 1;
        end
    end
    if(x_t == 0)
        neighb_array(y,index+1) = x;
    end
    if(z_t == 0)
        neighb_array(y,index+2-x_t) = z;
    end
    
    %neighbors of z
    index = max(find(neighb_array(z,:)));
    x_t = 0;
    y_t = 0;
    for j = 2:index
        if(neighb_array(z,j) == x)
            x_t = 1;
        end
        if(neighb_array(z,j) == y)
            y_t = 1;
        end
    end
    if(x_t == 0)
        neighb_array(z,index+1) = x;
    end
    if(y_t == 0)
        neighb_array(z,index+2-x_t) = y;
    end
end

    
    