% input: i = index to one mass
% input: j = index to neighbor mass
% input: nCols = number of columns
% output: d = direction, used for connection list index
% output: c = connection index
function [d, c] = massIndices2connectionIndices(i,j,nCols)
d = -1;
c = -1;

if(abs(i-j) == 1)
    % connection orientation: -
    if rem(min(i,j),nCols) ~= 0
        d = 3;
        c = min(i,j) - floor(min(i,j)/nCols);
    else
        return
    end
elseif(abs(i-j) == nCols-1)
    % connection orientation: /
    if rem(min(i,j)-1,nCols) ~= 0
        d = 2;
        c = min(i,j) - floor(max(i,j)/nCols);
    end
elseif(abs(i-j) == nCols)
    % connection orientation: |
    if rem(min(i,j)-1,nCols) ~= 0
        d = 1;
        c = min(i,j);
    end
elseif(abs(i-j) == nCols+1 )
    % connection orientation: \
    if rem(min(i,j),nCols) ~= 0
        d = 4;
        c = min(i,j) - ceil(min(i,j)/nCols) + 1;
    end
else
    error(['|i-j| = ', num2str(abs(i-j)) ' -> No such neighbor']);
end

end
