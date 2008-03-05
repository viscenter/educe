function neighb_set=find_neighb(node_set, neighb_array)
%neighb_set=find_neighb(node_set, neighb_array)
    neighb_set=[];
    for i=1:length(node_set)
%        for j=1:neighb_array(node_set(i),2)
        %
        %  MY EDIT - ANDREW
        %
      
        for j = 2:max(find(neighb_array(node_set(i),:)))
            if ~ismember(neighb_array(node_set(i),j),[node_set neighb_set])
                %neighb_set=[neighb_set neighb_array(node_set(i),2+j)];
                neighb_set=[neighb_set neighb_array(node_set(i),j)];
            end
        end
    end
              
              
    