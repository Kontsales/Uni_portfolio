library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Simple counter used to count the time the player has to press a button during the game
-- The time can be changed in the top module to lower or increase the difficulty.
-- when the time is up the "timeIsUp" output signal is set as '1' 

entity counter is
    generic (timeToPress : integer);    
    Port (  clk : in STD_LOGIC;         -- system clock as input
            rst : in STD_LOGIC;
            timeIsUp : out STD_LOGIC := '0'
    );
end counter;

architecture Behavioral of counter is
    signal tick : integer;
    
begin

count: process(clk)
begin
    if rising_edge(clk) then
        if rst = '1' then       -- reset state
            tick <= 0;          
            timeIsUp <= '0';
        else
            -- true if counted to the timeToPress
            if tick = timeToPress - 1 then
                timeIsUp <= '1';        -- set output as '1'
            else
                tick <= tick + 1;       -- count up 1.
            end if;
        end if;   
    end if;
end process;   

end Behavioral;
