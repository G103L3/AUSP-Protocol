/*! \file frequencies_comparator.c
 * \author Gioele Giunta
 * \version 1.4
 * \since 21<sup>st</sup> March 2024
 * \brief Functions for frequencies_comparator.h
 */
 
 
/* Header */
#include "frequencies_comparator.h"

/* Function Headers */
/* + Assembly */
int compare_higher_frequency(int *higher_frequency);
int compare_lower_frequency(int *higher_frequency);
/* + C */
int add_char_to_sequence(char *sequence, char *recognized_key);

/*! \fn int frequencies_comparator(struct_tone_frequencies tone_frequencies, char *last_char, char *sequence)
* \param tone_frequencies The struct containing the high and low frequencies
* \param *last_char The pointer to the last_char, used to determine if there is a repetition or not
* \param *sequence The char Array where character will be saved
* \returns The status of operation 1 -> Tone detected, 0 -> Noise Detected/No operation by main required, <0 -> Error detected Operations from main Required
* \brief Compares the input frequencies to the DTMF frequency matrix and determines the corresponding key.
*
* This function takes in a struct containing the high and low frequencies of a DTMF signal, pointers to the last character detected and the character sequence. 
* It then determines the key that corresponds to the input frequencies and adds it to the character sequence, if it is a new character, also handles the case where OVERFLOW is detected. 
* The function also handles the case where no DTMF signal is detected or multiple DTMF signals are detected.
*/
int frequencies_comparator(struct_tone_frequencies tone_frequencies, char *last_char, char *sequence)
{
    char DTMF_keys_matrix[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
    int column, row;
    char recognized_key;
		
    if(tone_frequencies.high == 0 || tone_frequencies.low == 0)
	/* Noise Detected */
	{
      	*last_char = 'N'; 
		/* Case where no DTMF signal has been detected */
		return 0;
	}
	
	if(tone_frequencies.high == -1 || tone_frequencies.low == -1)
		/* Multitone Detected */
	{ 
      *last_char = 'N'; 
		/* Set MULTITONE ERROR */
		//strcpy(error_text, "MULTITONE!");
		/* Case where more than one DTMF signal has been detected */
		return -2;
	}
		
		/* Call the Assembly Functions to obtain the keys of the DTMF matrix */
    column = compare_higher_frequency(&tone_frequencies.high);
    row = compare_lower_frequency(&tone_frequencies.low);

    if(column == -1 || row == -1)
	/* Noise Detected */
	{
        *last_char = 'N';
        return 0;
    }
		else
		{
				/* Find in the matrix the right tone using row and column */
        recognized_key = DTMF_keys_matrix[row][column];
        if(recognized_key != *last_char)
		{
			if(add_char_to_sequence(sequence, &recognized_key))
			{
				/* Character successfully added to sequence, proceede memorizing last_char */
				*last_char = recognized_key;
				return 1;
			}
			else
			{
				/* Throw error the addChar returned 0 so Overflow occured Error: -1 */
				return -1; 
			}
        }
		else
		{
            /* In case of last_char repetition no operations! */
            return 0;
        }
    }
}

/*! \fn int add_char_to_sequence(char *sequence, char *recognized_key)
* \brief Adds a character to the character sequence.
*
* This function takes in a pointer to the character sequence and a pointer to the character to be added. It then adds the character to the sequence if there is space available, and returns the status of the operation.
* @param sequence The Array Address
* @param *recognized_key The pointer to the charater to add to the array
* @returns The status of operation 1 -> true, 0 -> Overflow Occured
*/
int add_char_to_sequence(char *sequence, char *recognized_key) 
{
    int array_length = strlen(sequence);
    if (array_length < G_SEQUENCE_LENGTH-2) 
	{
		sequence[array_length] = *recognized_key;
		sequence[array_length + 1] = '\0';
      return 1;
    }
		else
		{
			/* Set OVERFLOW ERROR */
			//strcpy(error_text, "OVERFLOW!");
			/* Case where the sequence is in Overflow */
			return 0;
		}
}

/*! \fn int compare_higher_frequency(int *higher_frequency)
* \brief Compares the higher frequency [1209, 1336, 1477, 1633] of keys [1,2,3,4] and returns the key.
*
* This function takes in a pointer to the higher frequency value and compares it to the predefined DTMF higher frequencies. It then returns the corresponding key.
*
* @param higher_frequency The address of the higher frequency value.
* @returns The key corresponding to the higher frequency value.
*/
int compare_higher_frequency(int *higher_frequency) {
  switch (*higher_frequency) {
      case 1209: return 0;
      case 1336: return 1;
      case 1477: return 2;
      case 1633: return 3;
      default: return -1;
  }
}


/*! \fn int compare_lower_frequency(int *lower_frequency)
* \brief Compares the lower frequency [697, 770, 852, 941] of keys [1,2,3,4] and returns the key.
* This function takes in a pointer to the lower frequency value and compares it to the predefined DTMF lower frequencies. It then returns the corresponding key.
*
* @param lower_frequency The address of the lower frequency value.
* @returns The key corresponding to the lower frequency value.
*/
int compare_lower_frequency(int *lower_frequency) {
  switch (*lower_frequency) {
      case 697: return 0;
      case 770: return 1;
      case 852: return 2;
      case 941: return 3;
      default: return -1;
  }
}


// ******************************* Gioele Giunta University Of Malta *************************************
