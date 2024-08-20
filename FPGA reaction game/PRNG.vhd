library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- This pseudo-random number generator is used to choose which led to light.
-- It runs in the background during the game and the "rngOut" is saved to a variable in "top"
-- whenever a new led is needed.

-- Basically it is a 4-bit linear-feedback shift register but instead of outputting '1' or '0'
-- it sends out a 4-bit logic vector each clock cycle based on the state of the register.
-- Only has 15 states, so the chances for the leds to light are not equal, but it works well enough.
-- bits could be increased to make it fairer.


entity PRNG is
    Port (  clk: in STD_LOGIC;      -- system clock as input
            rst: in STD_LOGIC;      
            rngOut : out std_logic_vector (3 downto 0) := (others => '0')
            );        
    constant seed: std_logic_vector (3 downto 0) := "0001";     -- seed for the prng
end PRNG;

architecture Behavioral of PRNG is
    signal qt: std_logic_vector (3 downto 0) := seed;       -- start the register at the seed     
begin

prng: process(clk)
    variable tmp : STD_LOGIC := '0';    -- store the 'xor':d value temporarily
begin
    if rising_edge(clk) then
        if rst = '1' then       
            qt <= seed;                     -- reset sets the register to the seed
            rngOut <= "0000";               -- no leds light if you dont want to
        else
            tmp := qt(1) xor qt(0);         -- take the 'xor' of the last two bits of the register
            qt <= tmp & qt (3 downto 1);    -- set the leftmost value to tmp, and shift the register 1 bit to right.
                
                -- checking the register and based on it send out a signal which lights a single led.
                if qt < "0101" then         
                    rngOut <= "0001";
                elsif qt < "1001" then
                    rngOut <= "0010";
                elsif qt < "1101" then
                    rngOut <= "0100";
                elsif qt > "1101" or qt = "1101" then
                    rngOut <= "1000";
                end if;
        end if;                        
    end if;
end process;   
  
end Behavioral;
