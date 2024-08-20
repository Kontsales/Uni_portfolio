library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- this module takes in the score the player has attained and sends out a blinking signal based on that score.
-- the blink frequency can be changed in the top module. 
-- Works as a simple counter that sends out 4-bits alternating "0000" and "1111".

entity BlinkyBlonk is
    generic(blinkfreq : integer);
    Port (  score : in integer;
            clk : in STD_LOGIC;
            rst : in STD_LOGIC;
            blinkDone : out std_logic;
            blink : out STD_LOGIC_VECTOR (3 downto 0));
end BlinkyBlonk;

architecture Behavioral of BlinkyBlonk is
    signal tick : integer;              -- counting signal
    signal flipFlop : std_logic;        -- tells if leds are on or off
    signal counter: integer := 0;       -- counts the times the leds are lit to compare with score
begin

blinkyblonk: process(clk)
begin
    if rising_edge(clk) then
        
        -- if reset, reset
        if rst = '1' then
            tick <= 0;
            Blink <= "0000";
            flipFlop <= '0';
            blinkDone <= '0';
            counter <= 0;
        
        else
            -- if counted to the time set
            if tick = blinkfreq/4 - 1 then
                
                -- if leds are off
                if flipFlop = '1' then
                    report "blink on";
                    blink <= "1111";            -- turn leds on
                    flipFlop <= '0';        
                    tick <= 0;
                    counter <= counter + 1;     -- count up 1 count
                
                -- if leds are on
                else
                    report "blink off";
                    blink <= "0000";        -- turn leds off
                    flipFlop <= '1';
                    tick <= 0;
                    -- if leds have been turned on and off for the amount of score
                    if counter = score then
                        blinkDone <= '1';
                    end if;
                end if;
            -- if not counted to the time set
            else
                tick <= tick + 1;       -- count up by one
            end if;
        end if;   
    end if;
end process;   

end Behavioral;
