

def format_number(number, width):
  """Formats a number with spaces for alignment.

  Args:
      number: The number to format (can be integer or float).
      width: The minimum width of the formatted string.

  Returns:
      A string with the formatted number, right-aligned and padded with spaces.
  """

  # Convert to integer for spacing
  number_int = int(number)
  formatted_string = f"{number_int:{width}d}"
  return formatted_string
