library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity top is
    Port ( btn : in STD_LOGIC_VECTOR (3 downto 0);
           led : out STD_LOGIC_VECTOR (3 downto 0);
           sysclk : in STD_LOGIC);
end top;

architecture Behavioral of top is
    
    constant timeToPress: integer := 125e6;     -- Change this to set the difficulty of the game:
                                                -- time to press a button = (difficulty / 125 000 000) [s]
    
    constant blinkfreq: integer := 125e6;       -- Clockfrequency for the blinkyblonker (GameOver score indicator):
                                                -- blinking period = (clkfreq / 2 * 125 000 000) [s]
                                                                                               
    constant delay: integer := 50000;           -- change this to change the debounce counter time.
    
    type t_state is (init,              -- States for the finite state machine
                    LogicUpdate, 
                    WaitForButton, 
                    GotButton, 
                    GameOver);    
    signal state: t_state := init;      -- start at "init" state

    signal timeIsUp: std_logic := '0';                              -- signal from the counter to indicate that time is up
    signal timerRst, rngRst, dbRst, blinkRst: std_logic := '1';     -- reset signals for all the entities (all resets are active high)
    signal rngOut: std_logic_vector (3 downto 0);                   -- signal from the prng to choose which led to light
    signal debounce: std_logic_vector (3 downto 0);                 -- output signal from the debouncer to indicate a true button press
    signal blink : std_logic_vector (3 downto 0);                   -- signal from the blinkyblonk to blink leds at the end of the game
    signal blinkDone: std_logic;                                    -- signal from the blinkyblonk to indicate blinking has ended
    signal score: integer range 0 to 255;                           -- signal to count the score. used to determine how many times leds will blink at the end of the game.
    
begin
-- instantiate the pseudo-random number generator, timer, button debouncer and the gameOver score indicator.
    prng: entity work.PRNG port map(clk => sysclk, rst => rngRst, rngOut => rngOut);
    counter: entity work.counter generic map(timeToPress => timeToPress) port map(clk => sysclk, rst => timerRst, timeIsUp => timeIsUp);
    debouncer: entity work.debouncer generic map (delay => delay) port map(clk => sysclk, reset => dbRst, btn => btn, debounce => debounce);
    BlinkyBlonk: entity work.BlinkyBlonk generic map(blinkfreq => blinkfreq) port map(score => score, clk => sysclk, rst => blinkRst, blinkDone => blinkDone, blink => blink);

-- top process
gameState: process(sysclk)
    variable nextLed: std_logic_vector (3 downto 0);        -- variable to store the random value from the pseudo-random number generator
    variable tmpBtn: std_logic_vector (3 downto 0);         -- variable to store the button press for checking if its correct
        
begin
    if rising_edge(sysclk) then
        case(state) is        
            
            -- setup state
            when init =>            
                led <= "0000";      -- set all leds on
                timerRst <= '1';    -- set timer off
                dbRst <= '0';       -- set debouncer to start sampling
                rngRst <= '0';      -- set prng on
                score <= 0;         -- reset the score
                -- Wait for player to start by pressing btn(0)
                if debounce(0) = '1' then          
                    state <= LogicUpdate;
                end if;
                
            -- LogicUpdate state       
            when LogicUpdate =>
                nextLed := rngOut;          -- store a value from the LFSR output to a variable
                timerRst <= '0';            -- start the timer          
                state <= WaitForButton;     -- go to "WaitForButton" state to wait for the user to press a button
                
            -- WaitForButton state
            when WaitForButton =>
                led <= nextLed;                 --  use the stored variable to light up a led    
                -- true when the timer counts to x seconds.                
                if timeIsUp = '1' then          
                    report "ran out of time";
                    timerRst <= '1';            -- stop the timer
                    state <= GameOver;          -- player was too slow and the game is over    
                end if;
                -- true if a debounced button press is detected                  
                if debounce(0) = '1' or debounce(1) = '1' or debounce(2) = '1' or debounce(3) = '1'  then
                    tmpBtn := debounce;             -- save the button pressed to a variable
                    state <= GotButton;             
                end if;
            
            -- GotButton state                       
            when GotButton =>
                led <= "0000";                -- turn leds off
                timerRst <= '1';              -- stop the timer
                -- true if the button pressed was correct (same as the led that was lit)
                if tmpBtn = nextLed then
                    report "pressed correct"; 
                    score <= score + 1;       -- increase score by one
                    state <= LogicUpdate;     -- go back to "LogicUpdate" state
                -- if the player pressed the wrong button
                else                          
                    report "pressed wrong";  
                    state <= GameOver;        -- go to the "GameOver" state
                end if;
            
            -- GameOver state
            when GameOver =>
                blinkRst <= '0';            -- start the blinking process
                led <= blink;               -- (blinks for the amount of score attained)                                            
                -- true if the blinking is done                            
                if blinkDone = '1' then
                    blinkRst <= '1';        -- stop the blinking process
                    state <= init;
                end if;         
        end case;
    end if;
end process;

end Behavioral;
