#include <ios>
#include <iostream>
#include <termios.h> //tcsetattr
#include <unistd.h> //STDIN_FILENO
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

struct User
{
  int m_id{0};
  std::string m_username;
  std::string m_password;
};


void clearExtra()
{
  // clears extraneous input in the buffer
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// print the last time a file was modified
void get_lst_write_time(const std::filesystem::path& entry)
{
  auto ftime = std::filesystem::last_write_time(entry);
  std::time_t cftime = 
    std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
  std::cout << std::asctime(std::localtime(&cftime)) << '\n';
}


int getch()
{
  int ch;
  struct termios oldt, newt;

  tcgetattr(STDIN_FILENO, &oldt); // gets termios struct which contains control info for a terminal associated with a file descriptor
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO); // set input characters to not ECHO to the terminal and 
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // hide input

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // unhide input
  return ch;
}


std::string getpass(const char *prompt, bool show_asterisk=true)
{
  const char BACKSPACE = 127;
  const char RETURN = 10;

  std::string _password;
  unsigned char ch = 0;

  std::cout << prompt;

  while((ch = getch()) != RETURN) // as long as user doesn't press ENTER (RETURN), continue loop
  {
    if(ch == BACKSPACE) // if backspace is pressed, instead of adding a new char to the password, we delete the previous one instead
    {
      if(_password.length() != 0)
      {
        if(show_asterisk)
        {
          std::cout << "\b \b";
          _password.resize(_password.length() - 1);
        }
      }
    } 
    // hide password with '*'
    else
    {
      _password += ch;
      if(show_asterisk)
        std::cout << '*';
    }
  }
  std::cout << '\n';
  return _password;
}

void saveToFile(User &new_user)
{
  std::string usr_file_title = new_user.m_username + ".txt";

  std::ofstream db_file;
  db_file.open("database/" + usr_file_title);
  if(db_file.is_open())
  {
    db_file << new_user.m_username << '\n';
    db_file << new_user.m_password << '\n';

    db_file.close();
  }
  else
    std::cerr << "Error: could not open file \n";
}


std::string searchFile(std::filesystem::path path, std::string &usn)
try
{
  std::string _cred;
  if(std::filesystem::is_directory(path))
  {
    for(const std::filesystem::directory_entry& entr : std::filesystem::directory_iterator(path))
    {
      const std::filesystem::path& pth = entr;
      if(pth.stem() == usn)
      {
        std::cout << "Found user file: " << pth << '\n';
        std::ifstream _temp_file(pth);
        if(_temp_file.is_open())
        {
          std::string _line;
          while(std::getline(_temp_file, _line))
          {
            _cred = (static_cast<std::string>(_line.c_str()));
          }
          return _cred;
          _temp_file.close();
        }
      } 
    }
  }
}
catch(const std::filesystem::filesystem_error& ex)
{
  std::cerr << ex.what() << '\n';
}


void login()
{
  std::string _username, _passwd;
  int _login_count{0};
  bool _logged_in = false;

  while(!_logged_in)
  {
    std::cout << "Login: \n" << "User: ";
    std::cin >> _username;
    clearExtra();
    std::cout << "Password: ";
    _passwd = getpass("Enter your password: ", true);
    
    std::string _usr_pswd{searchFile("database",_username)};
    if(_usr_pswd == _passwd)
    {
      std::cout << "Login successful, welcome back :3 \n";
      _logged_in = true;
      break;
    }
    else
      std::cout << "Error: wrong username or password, try again \n";
  }
}

void registration()
{
  User _user;
  std::cout << "Enter your new username: ";
  std::cin >> _user.m_username;
  clearExtra();
  while(true)
  {
    std::string _pass1 = getpass("Enter your password: ", true);
    std::string _pass2 = getpass("Enter your password again: ", true);
    if(_pass1 == _pass2)
    {
      _user.m_password = _pass1;
      std::cout << "Password created successfully \n";
      break;
    }
    else
      std::cout << "Error: Passwords do not match, try again. \n";
  }
  _user.m_id += 1;
  _user.m_username += std::to_string(_user.m_id);
  std::cout << "User created successfully, welcome :3 \n";
  saveToFile(_user);

  std::string _temp_pathname {"database/" + _user.m_username + ".txt"};
  std::filesystem::path _temp_entry{_temp_pathname};
  std::cout << "pathname: " << _temp_pathname << "| last write time: ";
  get_lst_write_time(_temp_entry);
  std::cout << _user.m_username << '\n' << _user.m_password << '\n' << _user.m_id <<'\n';
}


int main()
{
  registration();
  login();
  return 0;
}
