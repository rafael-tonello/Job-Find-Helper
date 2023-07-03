#include "utils.h"

mutex Utils::names_locks_mutexes_mutex;
map<string, timed_mutex*> Utils::names_locks_mutexes;
bool Utils::srandOk = false;

void Utils::named_lock(string session_name, named_lock_f f, int timeout_ms)
{
    Utils::names_locks_mutexes_mutex.lock();
    if (Utils::names_locks_mutexes.count(session_name) <= 0)
    {
        Utils::names_locks_mutexes[session_name] = new timed_mutex();
    }
    Utils::names_locks_mutexes_mutex.unlock();

    if (timeout_ms > 0)
        Utils::names_locks_mutexes[session_name]->try_lock_for(std::chrono::milliseconds(timeout_ms));
    else
        Utils::names_locks_mutexes[session_name]->lock();
    try
    {
        f();
        Utils::names_locks_mutexes[session_name]->unlock();
    }
    catch(...)
    {
        Utils::names_locks_mutexes[session_name]->unlock(); 
        throw;
    }


}

void Utils::named_lock_forceunlock(string session_name)
{
    if (Utils::names_locks_mutexes.count(session_name))
        Utils::names_locks_mutexes[session_name]->unlock();
}

int64_t Utils::getCurrentTimeMicroseconds()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t Utils::getCurrentTimeMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t Utils::getCurrentTimeSeconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

string Utils::StringToHex(string &input, size_t size)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    size_t outputSize = size * 2;
    output.reserve(outputSize);
    for (auto count = 0; count < size; count++)
    {
        auto c = input[count];
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

string Utils::StringToHex(string& input) 
{
    return StringToHex(input, input.size());

}

string Utils::charVecToHex(char* data, size_t size)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    size_t outputSize = size * 2;
    output.reserve(outputSize);
    for (auto count = 0; count < outputSize; count++)
    {
        auto c = data[count];
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

string Utils::charVecToHex(const char* data, size_t size)
{
    return charVecToHex((char*)data, size);
}

vector<string> Utils::splitString(string source, string split_by)
{
    vector<string> result;
    
    while (true)
    {
        if (auto pos = source.find(split_by); pos != string::npos)
        {
            result.push_back(source.substr(0, pos));
            source = source.substr(pos + split_by.size());
        }
        else
            break;
    }

    if (source != "")
        result.push_back(source);

    return result;
}

string Utils::strToUpper(std::string source)
{
    std::string result = source;

    for (char& c : result) {
        c = std::toupper(c);
    }

    return result;
}

string Utils::strToLower(std::string source)
{
    std::string result = source;

    for (char& c : result) {
        c = std::tolower(c);
    }

    return result;
}


string Utils::getOnly(string source, string validChars)
{
    string ret = "";
    for (auto &c: source)
        if (validChars.find(c) != string::npos)
            ret.push_back(c);

    return ret;
}

string Utils::ssystem(string command, bool removeTheLastLF) {

    const int bufferSize = 128;
    char buffer[bufferSize];
    std::string output;

    // Executar o comando e redirecionar stdout e stderr para o mesmo arquivo temporário
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (pipe == nullptr) {
        std::cerr << "Erro ao executar o comando." << std::endl;
        return output;
    }

    // Ler a saída do subprocesso (stdout e stderr)
    while (fgets(buffer, bufferSize, pipe) != nullptr) {
        output += buffer;
    }

    // Fechar o subprocesso
    pclose(pipe);

    // Remover a quebra de linha final, se necessário
    if (removeTheLastLF && !output.empty() && output.back() == '\n') {
        output.pop_back();
    }

    return output;
}

std::string Utils::ssystem2(string command, int timeout_milliseconds, bool removeTheLastLF) {
    int stdoutPipe[2];  // Pipe para a saída padrão (stdout)
    int stderrPipe[2];  // Pipe para a saída de erro (stderr)

    if (pipe(stdoutPipe) == -1 || pipe(stderrPipe) == -1) {
        std::cerr << "Erro ao criar pipes." << std::endl;
        return "";
    }

    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Erro ao criar processo filho." << std::endl;
        return "";
    } else if (pid == 0) {
        // Código do processo filho
        close(stdoutPipe[0]);  // Fechar o descritor de leitura do pipe de stdout
        close(stderrPipe[0]);  // Fechar o descritor de leitura do pipe de stderr

        // Redirecionar stdout para o pipe de escrita
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdoutPipe[1]);

        // Redirecionar stderr para o pipe de escrita
        dup2(stderrPipe[1], STDERR_FILENO);
        close(stderrPipe[1]);

        // Executar o comando
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        std::cerr << "Erro ao executar o comando." << std::endl;
        _exit(1);
    } else {
        // Código do processo pai
        close(stdoutPipe[1]);  // Fechar o descritor de escrita do pipe de stdout
        close(stderrPipe[1]);  // Fechar o descritor de escrita do pipe de stderr

        // Criar std::future para stdout e stderr
        std::future<std::string> stdoutFuture = std::async(std::launch::async, [&stdoutPipe]() {
            char buffer[128];
            std::string stdoutOutput;
            ssize_t bytesRead;

            while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer))) > 0) {
                stdoutOutput.append(buffer, bytesRead);
            }

            close(stdoutPipe[0]);  // Fechar o descritor de leitura do pipe de stdout

            return stdoutOutput;
        });

        std::future<std::string> stderrFuture = std::async(std::launch::async, [&stderrPipe]() {
            char buffer[128];
            std::string stderrOutput;
            ssize_t bytesRead;

            while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer))) > 0) {
                stderrOutput.append(buffer, bytesRead);
            }

            close(stderrPipe[0]);  // Fechar o descritor de leitura do pipe de stderr

            return stderrOutput;
        });

        // Definir o tempo limite usando std::future::wait_for()
        std::chrono::milliseconds timeout(timeout_milliseconds);
        auto startTime = std::chrono::steady_clock::now();
        bool timeoutOccurred = false;

        if (stdoutFuture.wait_for(timeout) == std::future_status::timeout ||
            stderrFuture.wait_for(timeout) == std::future_status::timeout) {
            // O tempo limite foi atingido
            timeoutOccurred = true;
            kill(pid, SIGTERM);  // Encerrar o processo filho
        }

        // Obter o resultado das saídas (stdout e stderr)
        std::string stdoutResult = stdoutFuture.get();
        std::string stderrResult = stderrFuture.get();

        // Remover o último caractere de nova linha, se necessário
        if (removeTheLastLF) {
            if (!stdoutResult.empty() && stdoutResult.back() == '\n') {
                stdoutResult.pop_back();
            }
            if (!stderrResult.empty() && stderrResult.back() == '\n') {
                stderrResult.pop_back();
            }
        }

        // Verificar se o tempo limite ocorreu
        if (timeoutOccurred) {
            //std::cerr << "Tempo limite excedido." << std::endl;
            stderrResult += "\nCommand timeout.";
            // Realizar ações adicionais em caso de tempo limite, se necessário
        }

        return stdoutResult + stderrResult;
    }
}

Utils::ProcessOutput Utils::ssystem3(const std::string& command) {
    int stdoutPipe[2];  // Pipe para a saída padrão (stdout)
    int stderrPipe[2];  // Pipe para a saída de erro (stderr)

    if (pipe(stdoutPipe) == -1 || pipe(stderrPipe) == -1) {
        std::cerr << "Erro ao criar pipes." << std::endl;
        return { -1, {}, {} };
    }

    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Erro ao criar processo filho." << std::endl;
        return { -1, {}, {} };
    } else if (pid == 0) {
        // Código do processo filho
        close(stdoutPipe[0]);  // Fechar o descritor de leitura do pipe de stdout
        close(stderrPipe[0]);  // Fechar o descritor de leitura do pipe de stderr

        // Redirecionar stdout para o pipe de escrita
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdoutPipe[1]);

        // Redirecionar stderr para o pipe de escrita
        dup2(stderrPipe[1], STDERR_FILENO);
        close(stderrPipe[1]);

        // Executar o comando
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        std::cerr << "Erro ao executar o comando." << std::endl;
        _exit(1);
    } else {
        // Código do processo pai
        close(stdoutPipe[1]);  // Fechar o descritor de escrita do pipe de stdout
        close(stderrPipe[1]);  // Fechar o descritor de escrita do pipe de stderr

        // Criar std::future para stdout e stderr
        std::future<std::string> stdoutFuture = std::async(std::launch::async, [&stdoutPipe]() {
            char buffer[128];
            std::string stdoutOutput;
            ssize_t bytesRead;

            while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer))) > 0) {
                stdoutOutput.append(buffer, bytesRead);
            }

            close(stdoutPipe[0]);  // Fechar o descritor de leitura do pipe de stdout

            return stdoutOutput;
        });

        std::future<std::string> stderrFuture = std::async(std::launch::async, [&stderrPipe]() {
            char buffer[128];
            std::string stderrOutput;
            ssize_t bytesRead;

            while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer))) > 0) {
                stderrOutput.append(buffer, bytesRead);
            }

            close(stderrPipe[0]);  // Fechar o descritor de leitura do pipe de stderr

            return stderrOutput;
        });

        return { pid, std::move(stdoutFuture), std::move(stderrFuture) };
    }
}

future<string> Utils::asystem(string command, bool removeTheLastLF)
{
    return std::async(std::launch::async, [&](string c, bool rtllf){
        return ssystem(c, rtllf);
    }, command, removeTheLastLF);
}

future<string> Utils::httpGet(string url, map<string, string> headers)
{
    string cmd = "curl -sS -k -X GET " +
    url + " "+
    "-H 'Accept: */*' ";
    for (auto &c: headers)
        cmd += "-H '"+c.first+": "+c.second+"' ";
    
    return std::async(std::launch::async, [](string cmd2){
        string ret = ssystem(cmd2);
        if (ret.find("curl") == string::npos)
            return ret;
        else
            throw std::runtime_error("Curl error: "+ret);

    }, cmd);

    

}

future<string> Utils::httpPost(string url, string body, string contentType, map<string, string> headers)
{
    string cmd = "curl -sS -k -X POST " +
    url + " "+
    "-H 'Accept: */*' " +
    "-H 'Content-Type: "+contentType+"' ";
    for (auto &c: headers)
        cmd += "-H '"+c.first+": "+c.second+"' " ;
    cmd += "-d '"+body+"'";

    return std::async(std::launch::async, [](string cmd2){
        string ret = ssystem(cmd2);
        if (ret.find("curl") == string::npos)
            return ret;
        else
            throw std::runtime_error("Curl error: "+ret);

    }, cmd);

}



void Utils::process_mem_usage(double& vm_usage, double& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}

double Utils::process_vm_usage()
{
    double a, b;
    process_mem_usage(a, b);
    return a;
}

double Utils::process_resident_usage()
{
    double a, b;
    process_mem_usage(a, b);
    return b;
}


void Utils::runSRand()
{
    if (Utils::srandOk == false)
    {
        srand((unsigned)time(0)); 
        Utils::srandOk = true;
    }
}

string Utils::createUniqueId(string validChars, int size, string prefix, string sufix, bool includeTimeStampAtBegining)
{

    Utils::runSRand();
    auto i = to_string(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    //auto i2 = to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    
    string i2 = "";


    int randomCharsCount = size - (includeTimeStampAtBegining ? i.size() : 0);
    for (int c = 0; c < randomCharsCount; c++)
    {
        i2 += validChars[rand() % validChars.size()];
    }

    string tmp = prefix + (includeTimeStampAtBegining ? i: "") + i2 + sufix;
    //return Utils::StringToHex(tmp);
    return tmp;

}

string Utils::createUniqueId_customFormat(string format, string prefix, string sufix)
{
    //auto chartList = Utils::createUniqueId(validChars, format.size(), "", "", includeTimeStampAtBegining);

    stringstream ret;

    Utils::runSRand();
    ret.clear();
    string chars = "0123456789abcdefghijklmnopqrstuvxywz";

    auto getAlpha = [chars](){ return chars[(rand() % (chars.size()-10))+10]; };
    auto getNum = [chars](){ return chars[rand() % 10]; };
    auto getHex = [chars](){ return chars[rand() % 16]; };
    auto getAlphaRandUpper = [getAlpha, chars](){
        if (((rand() % 2)+1) == 2)
            return (char)toupper(getAlpha());
        else
            return getAlpha();

    };

    
    for (size_t c = 0; c < format.size(); c++)
    {
        if (format[c] == 'a')
            ret << getAlpha();
        else if (format[c] == 'A')
            ret << (char)toupper(getAlpha());
        else if (format[c] == 'h')
            ret << getHex();
        else if (format[c] == 'H')
            ret << (char)toupper(getHex());
        else if (format[c] == '0')
            ret << getNum();
        else if (format[c] == '?')
            ret << (vector<function<char()>>({ getAlphaRandUpper, getNum, getHex}))[rand() % 3]();
        else
            ret << format[c];
    }
    string ret2 = ret.str();
    return prefix + ret2  + sufix;

}

string Utils::createUnidqueId_guidFormat()
{
    /*auto charList = Utils::createUniqueId("0123456789abcdef", 32, "", "", false);

    string ret = ""+
                    charList.substr(0, 8) +  "-"+
                    charList.substr(8, 4) +  "-"+
                    charList.substr(12, 4) + "-"+ 
                    charList.substr(16, 4) + "-"+ 
                    charList.substr(20); */
    string ret = Utils::createUniqueId_customFormat("hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh");
    return ret;
}



string Utils::readTextFileContent(string fileName)
{
    ifstream f;
    stringstream content;
    string result = "";
    f.open(fileName);
    if (f.is_open())
    {
        content << f.rdbuf();
        result =  content.str();
        f.close();
    }

    return result;
}

void Utils::writeTextFileContent(string fileName, string content)
{
    ofstream f(fileName);
    if (f.is_open())
    {
        f << content;
        f.close();
    }
}

void Utils::appendTextFileContent(string fileName, string content)
{
    ofstream f(fileName, std::ios_base::app);
    if (f.is_open())
    {
        f << content;
        f.close();
    }
}

// // string Utils::findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr)
// // {
// //     // Get the first occurrence
// //     size_t pos = data.find(toSearch);
// //     // Repeat till end is reached
// //     while( pos != std::string::npos)
// //     {
// //         // Replace this occurrence of Sub String
// //         data.replace(pos, toSearch.size(), replaceStr);
// //         // Get the next occurrence from the current position
// //         pos =data.find(toSearch, pos + replaceStr.size());
// //     }

// //     return data;
// // }

string Utils::stringReplace(string source, string replace, string by)
{
    stringstream ret;
    auto pos = source.find(replace);
    while (pos != string::npos)
    {
        ret << source.substr(0, pos) << by;
        source = source.substr(pos + replace.size());
        pos = source.find(replace);
    }
    ret << source;
    return ret.str();
}

string Utils::stringReplace(string source, vector<tuple<string, string>> replaceAndByTuples)
{
    for (auto &c: replaceAndByTuples)
        source = Utils::stringReplace(source, std::get<0>(c), std::get<1>(c));
    return source;
}

bool Utils::isNumber(string source)
{
    string validChars = "0123456789";
    bool alreadyDot = false;
    for (int c = 0; c < source.size(); c++)
    {
        char curr = source[c];
        if ((curr == '+' || curr == '-') && c != 0)
            return false;
        else if (curr == '.')
        {
            if (alreadyDot)
                return false;
            else
                alreadyDot = true;
        }
        else if (validChars.find(curr) == string::npos)
            return false;
    }

    return true;
}

map<void*, string> Utils::getANameDB;
//https://dzone.com/articles/name-lists-generating-test
string Utils::getAName(int number, NameType typeOfName, int AlgoGenMaxSyllables){
    return getAName((void*)number, typeOfName, AlgoGenMaxSyllables);
}

string Utils::getAName(void* p, NameType typeOfName, int AlgoGenMaxSyllables){
    vector<string> surnames_list={"Smith","Johnson","Williams","Brown","Jones","Miller","Davis","Garcia","Rodriguez","Wilson","Martinez","Anderson","Taylor","Thomas","Hernandez","Moore","Martin","Jackson","Thompson","White","Lopez","Lee","Gonzalez","Harris","Clark","Lewis","Robinson","Walker","Perez","Hall","Young","Allen","Sanchez","Wright","King","Scott","Green","Baker","Adams","Nelson","Hill","Ramirez","Campbell","Mitchell","Roberts","Carter","Phillips","Evans","Turner","Torres","Parker","Collins","Edwards","Stewart","Flores","Morris","Nguyen","Murphy","Rivera","Cook","Rogers","Morgan","Peterson","Cooper","Reed","Bailey","Bell","Gomez","Kelly","Howard","Ward","Cox","Diaz","Richardson","Wood","Watson","Brooks","Bennett","Gray","James","Reyes","Cruz","Hughes","Price","Myers","Long","Foster","Sanders","Ross","Morales","Powell","Sullivan","Russell","Ortiz","Jenkins","Gutierrez","Perry","Butler","Barnes","Fisher","Henderson","Coleman","Simmons","Patterson","Jordan","Reynolds","Hamilton","Graham","Kim","Gonzales","Alexander","Ramos","Wallace","Griffin","West","Cole","Hayes","Chavez","Gibson","Bryant","Ellis","Stevens","Murray","Ford","Marshall","Owens","Mcdonald","Harrison","Ruiz","Kennedy","Wells","Alvarez","Woods","Mendoza","Castillo","Olson","Webb","Washington","Tucker","Freeman","Burns","Henry","Vasquez","Snyder","Simpson","Crawford","Jimenez","Porter","Mason","Shaw","Gordon","Wagner","Hunter","Romero","Hicks","Dixon","Hunt","Palmer","Robertson","Black","Holmes","Stone","Meyer","Boyd","Mills","Warren","Fox","Rose","Rice","Moreno","Schmidt","Patel","Ferguson","Nichols","Herrera","Medina","Ryan","Fernandez","Weaver","Daniels","Stephens","Gardner","Payne","Kelley","Dunn","Pierce","Arnold","Tran","Spencer","Peters","Hawkins","Grant","Hansen","Castro","Hoffman","Hart","Elliott","Cunningham","Knight","Bradley","Carroll","Hudson","Duncan","Armstrong","Berry","Andrews","Johnston","Ray","Lane","Riley","Carpenter","Perkins","Aguilar","Silva","Richards","Willis","Matthews","Chapman","Lawrence","Garza","Vargas","Watkins","Wheeler","Larson","Carlson","Harper","George","Greene","Burke","Guzman","Morrison","Munoz","Jacobs","Obrien","Lawson","Franklin","Lynch","Bishop","Carr","Salazar","Austin","Mendez","Gilbert","Jensen","Williamson","Montgomery","Harvey","Oliver","Howell","Dean","Hanson","Weber","Garrett","Sims","Burton","Fuller","Soto","Mccoy","Welch","Chen","Schultz","Walters","Reid","Fields","Walsh","Little","Fowler","Bowman","Davidson","May","Day","Schneider","Newman","Brewer","Lucas","Holland","Wong","Banks","Santos","Curtis","Pearson","Delgado","Valdez","Pena","Rios","Douglas","Sandoval","Barrett","Hopkins","Keller","Guerrero","Stanley","Bates","Alvarado","Beck","Ortega","Wade","Estrada","Contreras","Barnett","Caldwell","Santiago","Lambert","Powers","Chambers","Nunez","Craig","Leonard","Lowe","Rhodes","Byrd","Gregory","Shelton","Frazier","Becker","Maldonado","Fleming","Vega","Sutton","Cohen","Jennings","Parks","Mcdaniel","Watts","Barker","Norris","Vaughn","Vazquez","Holt","Schwartz","Steele","Benson","Neal","Dominguez","Horton","Terry","Wolfe","Hale","Lyons","Graves","Haynes","Miles","Park","Warner","Padilla","Bush","Thornton","Mccarthy","Mann","Zimmerman","Erickson","Fletcher","Mckinney","Page","Dawson","Joseph","Marquez","Reeves","Klein","Espinoza","Baldwin","Moran","Love","Robbins","Higgins","Ball","Cortez","Le","Griffith","Bowen","Sharp","Cummings","Ramsey","Hardy","Swanson","Barber","Acosta","Luna","Chandler","Blair","Daniel","Cross","Simon","Dennis","Oconnor","Quinn","Gross","Navarro","Moss","Fitzgerald","Doyle","Mclaughlin","Rojas","Rodgers","Stevenson","Singh","Yang","Figueroa","Harmon","Newton","Paul","Manning","Garner","Mcgee","Reese","Francis","Burgess","Adkins","Goodman","Curry","Brady","Christensen","Potter","Walton","Goodwin","Mullins","Molina","Webster","Fischer","Campos","Avila","Sherman","Todd","Chang","Blake","Malone","Wolf","Hodges","Juarez","Gill","Farmer","Hines","Gallagher","Duran","Hubbard","Cannon","Miranda","Wang","Saunders","Tate","Mack","Hammond","Carrillo","Townsend","Wise","Ingram","Barton","Mejia","Ayala","Schroeder","Hampton","Rowe","Parsons","Frank","Waters","Strickland","Osborne","Maxwell","Chan","Deleon","Norman","Harrington","Casey","Patton","Logan","Bowers","Mueller","Glover","Floyd","Hartman","Buchanan","Cobb","French","Kramer","Mccormick","Clarke","Tyler","Gibbs","Moody","Conner","Sparks","Mcguire","Leon","Bauer","Norton","Pope","Flynn","Hogan","Robles","Salinas","Yates","Lindsey","Lloyd","Marsh","Mcbride","Owen","Solis","Pham","Lang","Pratt","Lara","Brock","Ballard","Trujillo","Shaffer","Drake","Roman","Aguirre","Morton","Stokes","Lamb","Pacheco","Patrick","Cochran","Shepherd","Cain","Burnett","Hess","Li","Cervantes","Olsen","Briggs","Ochoa","Cabrera","Velasquez","Montoya","Roth","Meyers","Cardenas","Fuentes","Weiss","Wilkins","Hoover","Nicholson","Underwood","Short","Carson","Morrow","Colon","Holloway","Summers","Bryan","Petersen","Mckenzie","Serrano","Wilcox","Carey","Clayton","Poole","Calderon","Gallegos","Greer","Rivas","Guerra","Decker","Collier","Wall","Whitaker","Bass","Flowers","Davenport","Conley","Houston","Huff","Copeland","Hood","Monroe","Massey","Roberson","Combs","Franco","Larsen","Pittman","Randall","Skinner","Wilkinson","Kirby","Cameron","Bridges","Anthony","Richard","Kirk","Bruce","Singleton","Mathis","Bradford","Boone","Abbott","Charles","Allison","Sweeney","Atkinson","Horn","Jefferson","Rosales","York","Christian","Phelps","Farrell","Castaneda","Nash","Dickerson","Bond","Wyatt","Foley","Chase","Gates","Vincent","Mathews","Hodge","Garrison","Trevino","Villarreal","Heath","Dalton","Valencia","Callahan","Hensley","Atkins","Huffman","Roy","Boyer","Shields","Lin","Hancock","Grimes","Glenn","Cline","Delacruz","Camacho","Dillon","Parrish","Oneill","Melton","Booth","Kane","Berg","Harrell","Pitts","Savage","Wiggins","Brennan","Salas","Marks","Russo","Sawyer","Baxter","Golden","Hutchinson","Liu","Walter","Mcdowell","Wiley","Rich","Humphrey","Johns","Koch","Suarez","Hobbs","Beard","Gilmore","Ibarra","Keith","Macias","Khan","Andrade","Ware","Stephenson","Henson","Wilkerson","Dyer","Mcclure","Blackwell","Mercado","Tanner","Eaton","Clay","Barron","Beasley","Oneal","Small","Preston","Wu","Zamora","Macdonald","Vance","Snow","Mcclain","Stafford","Orozco","Barry","English","Shannon","Kline","Jacobson","Woodard","Huang","Kemp","Mosley","Prince","Merritt","Hurst","Villanueva","Roach","Nolan","Lam","Yoder","Mccullough","Lester","Santana","Valenzuela","Winters","Barrera","Orr","Leach","Berger","Mckee","Strong","Conway","Stein","Whitehead","Bullock","Escobar","Knox","Meadows","Solomon","Velez","Odonnell","Kerr","Stout","Blankenship","Browning","Kent","Lozano","Bartlett","Pruitt","Buck","Barr","Gaines","Durham","Gentry","Mcintyre","Sloan","Rocha","Melendez","Herman","Sexton","Moon","Hendricks","Rangel","Stark","Lowery","Hardin","Hull","Sellers","Ellison","Calhoun","Gillespie","Mora","Knapp","Mccall","Morse","Dorsey","Weeks","Nielsen","Livingston","Leblanc","Mclean","Bradshaw","Glass","Middleton","Buckley","Schaefer","Frost","Howe","House","Mcintosh","Ho","Pennington","Reilly","Hebert","Mcfarland","Hickman","Noble","Spears","Conrad","Arias","Galvan","Velazquez","Huynh","Frederick","Randolph","Cantu","Fitzpatrick","Mahoney","Peck","Villa","Michael","Donovan","Mcconnell","Walls","Boyle","Mayer","Zuniga","Giles","Pineda","Pace","Hurley","Mays","Mcmillan","Crosby","Ayers","Case","Bentley","Shepard","Everett","Pugh","David","Mcmahon","Dunlap","Bender","Hahn","Harding","Acevedo","Raymond","Blackburn","Duffy","Landry","Dougherty","Bautista","Shah","Potts","Arroyo","Valentine","Meza","Gould","Vaughan","Fry","Rush","Avery","Herring","Dodson","Clements","Sampson","Tapia","Bean","Lynn","Crane","Farley","Cisneros","Benton","Ashley","Mckay","Finley","Best","Blevins","Friedman","Moses","Sosa","Blanchard","Huber","Frye","Krueger","Bernard","Rosario","Rubio","Mullen","Benjamin","Haley","Chung","Moyer","Choi","Horne","Yu","Woodward","Ali","Nixon","Hayden","Rivers","Estes","Mccarty","Richmond","Stuart","Maynard","Brandt","Oconnell","Hanna","Sanford","Sheppard","Church","Burch","Levy","Rasmussen","Coffey","Ponce","Faulkner","Donaldson","Schmitt","Novak","Costa","Montes","Booker","Cordova","Waller","Arellano","Maddox","Mata","Bonilla","Stanton","Compton","Kaufman","Dudley","Mcpherson","Beltran","Dickson","Mccann","Villegas","Proctor","Hester","Cantrell","Daugherty","Cherry","Bray","Davila","Rowland","Madden","Levine","Spence","Good","Irwin","Werner","Krause","Petty","Whitney","Baird","Hooper","Pollard","Zavala","Jarvis","Holden","Haas","Hendrix","Mcgrath","Bird","Lucero","Terrell","Riggs","Joyce","Mercer","Rollins","Galloway","Duke","Odom","Andersen","Downs","Hatfield","Benitez","Archer","Huerta","Travis","Mcneil","Hinton","Zhang","Hays","Mayo","Fritz","Branch","Mooney","Ewing","Ritter","Esparza","Frey","Braun","Gay","Riddle","Haney","Kaiser","Holder","Chaney","Mcknight","Gamble","Vang","Cooley","Carney","Cowan","Forbes","Ferrell","Davies","Barajas","Shea","Osborn","Bright","Cuevas","Bolton","Murillo","Lutz","Duarte","Kidd","Key","Cooke"};
    vector<string> male_names_list={"James","John","Robert","Michael","William","David","Richard","Charles","Joseph","Thomas","Christopher","Daniel","Paul","Mark","Donald","George","Kenneth","Steven","Edward","Brian","Ronald","Anthony","Kevin","Jason","Matthew","Gary","Timothy","Jose","Larry","Jeffrey","Frank","Scott","Eric","Stephen","Andrew","Raymond","Gregory","Joshua","Jerry","Dennis","Walter","Patrick","Peter","Harold","Douglas","Henry","Carl","Arthur","Ryan","Roger","Joe","Juan","Jack","Albert","Jonathan","Justin","Terry","Gerald","Keith","Samuel","Willie","Ralph","Lawrence","Nicholas","Roy","Benjamin","Bruce","Brandon","Adam","Harry","Fred","Wayne","Billy","Steve","Louis","Jeremy","Aaron","Randy","Howard","Eugene","Carlos","Russell","Bobby","Victor","Martin","Ernest","Phillip","Todd","Jesse","Craig","Alan","Shawn","Clarence","Sean","Philip","Chris","Johnny","Earl","Jimmy","Antonio","Danny","Bryan","Tony","Luis","Mike","Stanley","Leonard","Nathan","Dale","Manuel","Rodney","Curtis","Norman","Allen","Marvin","Vincent","Glenn","Jeffery","Travis","Jeff","Chad","Jacob","Lee","Melvin","Alfred","Kyle","Francis","Bradley","Jesus","Herbert","Frederick","Ray","Joel","Edwin","Don","Eddie","Ricky","Troy","Randall","Barry","Alexander","Bernard","Mario","Leroy","Francisco","Marcus","Micheal","Theodore","Clifford","Miguel","Oscar","Jay","Jim","Tom","Calvin","Alex","Jon","Ronnie","Bill","Lloyd","Tommy","Leon","Derek","Warren","Darrell","Jerome","Floyd","Leo","Alvin","Tim","Wesley","Gordon","Dean","Greg","Jorge","Dustin","Pedro","Derrick","Dan","Lewis","Zachary","Corey","Herman","Maurice","Vernon","Roberto","Clyde","Glen","Hector","Shane","Ricardo","Sam","Rick","Lester","Brent","Ramon","Charlie","Tyler","Gilbert","Gene","Marc","Reginald","Ruben","Brett","Angel","Nathaniel","Rafael","Leslie","Edgar","Milton","Raul","Ben","Chester","Cecil","Duane","Franklin","Andre","Elmer","Brad","Gabriel","Ron","Mitchell","Roland","Arnold","Harvey","Jared","Adrian","Karl","Cory","Claude","Erik","Darryl","Jamie","Neil","Jessie","Christian","Javier","Fernando","Clinton","Ted","Mathew","Tyrone","Darren","Lonnie","Lance","Cody","Julio","Kelly","Kurt","Allan","Nelson","Guy","Clayton","Hugh","Max","Dwayne","Dwight","Armando","Felix","Jimmie","Everett","Jordan","Ian","Wallace","Ken","Bob","Jaime","Casey","Alfredo","Alberto","Dave","Ivan","Johnnie","Sidney","Byron","Julian","Isaac","Morris","Clifton","Willard","Daryl","Ross","Virgil","Andy","Marshall","Salvador","Perry","Kirk","Sergio","Marion","Tracy","Seth","Kent","Terrance","Rene","Eduardo","Terrence","Enrique","Freddie","Wade"};
    vector<string> female_names_list={"Mary","Patricia","Linda","Barbara","Elizabeth","Jennifer","Maria","Susan","Margaret","Dorothy","Lisa","Nancy","Karen","Betty","Helen","Sandra","Donna","Carol","Ruth","Sharon","Michelle","Laura","Sarah","Kimberly","Deborah","Jessica","Shirley","Cynthia","Angela","Melissa","Brenda","Amy","Anna","Rebecca","Virginia","Kathleen","Pamela","Martha","Debra","Amanda","Stephanie","Carolyn","Christine","Marie","Janet","Catherine","Frances","Ann","Joyce","Diane","Alice","Julie","Heather","Teresa","Doris","Gloria","Evelyn","Jean","Cheryl","Mildred","Katherine","Joan","Ashley","Judith","Rose","Janice","Kelly","Nicole","Judy","Christina","Kathy","Theresa","Beverly","Denise","Tammy","Irene","Jane","Lori","Rachel","Marilyn","Andrea","Kathryn","Louise","Sara","Anne","Jacqueline","Wanda","Bonnie","Julia","Ruby","Lois","Tina","Phyllis","Norma","Paula","Diana","Annie","Lillian","Emily","Robin","Peggy","Crystal","Gladys","Rita","Dawn","Connie","Florence","Tracy","Edna","Tiffany","Carmen","Rosa","Cindy","Grace","Wendy","Victoria","Edith","Kim","Sherry","Sylvia","Josephine","Thelma","Shannon","Sheila","Ethel","Ellen","Elaine","Marjorie","Carrie","Charlotte","Monica","Esther","Pauline","Emma","Juanita","Anita","Rhonda","Hazel","Amber","Eva","Debbie","April","Leslie","Clara","Lucille","Jamie","Joanne","Eleanor","Valerie","Danielle","Megan","Alicia","Suzanne","Michele","Gail","Bertha","Darlene","Veronica","Jill","Erin","Geraldine","Lauren","Cathy","Joann","Lorraine","Lynn","Sally","Regina","Erica","Beatrice","Dolores","Bernice","Audrey","Yvonne","Annette","June","Samantha","Marion","Dana","Stacy","Ana","Renee","Ida","Vivian","Roberta","Holly","Brittany","Melanie","Loretta","Yolanda","Jeanette","Laurie","Katie","Kristen","Vanessa","Alma","Sue","Elsie","Beth","Jeanne","Vicki","Carla","Tara","Rosemary","Eileen","Terri","Gertrude","Lucy","Tonya","Ella","Stacey","Wilma","Gina","Kristin","Jessie","Natalie","Agnes","Vera","Willie","Charlene","Bessie","Delores","Melinda","Pearl","Arlene","Maureen","Colleen","Allison","Tamara","Joy","Georgia","Constance","Lillie","Claudia","Jackie","Marcia","Tanya","Nellie","Minnie","Marlene","Heidi","Glenda","Lydia","Viola","Courtney","Marian","Stella","Caroline","Dora","Jo","Vickie","Mattie","Terry","Maxine","Irma","Mabel","Marsha","Myrtle","Lena","Christy","Deanna","Patsy","Hilda","Gwendolyn","Jennie","Nora","Margie","Nina","Cassandra","Leah","Penny","Kay","Priscilla","Naomi","Carole","Brandy","Olga","Billie","Dianne","Tracey","Leona","Jenny","Felicia","Sonia","Miriam","Velma","Becky","Bobbie","Violet","Kristina","Toni","Misty","Mae","Shelly","Daisy","Ramona","Sherri","Erika","Katrina","Claire","Lindsey","Lindsay","Geneva","Guadalupe","Belinda","Margarita","Sheryl","Cora","Faye","Ada","Natasha","Sabrina","Isabel","Marguerite","Hattie","Harriet","Molly","Cecilia","Kristi","Brandi","Blanche","Sandy","Rosie","Joanna","Iris","Eunice","Angie","Inez","Lynda","Madeline","Amelia","Alberta","Genevieve","Monique","Jodi","Janie","Maggie","Kayla","Sonya","Jan","Lee","Kristine","Candace","Fannie","Maryann","Opal","Alison","Yvette","Melody","Luz","Susie","Olivia","Flora","Shelley","Kristy","Mamie","Lula","Lola","Verna","Beulah","Antoinette","Candice","Juana","Jeannette","Pam","Kelli","Hannah","Whitney","Bridget","Karla","Celia","Latoya","Patty","Shelia","Gayle","Della","Vicky","Lynne","Sheri","Marianne","Kara","Jacquelyn","Erma","Blanca","Myra","Leticia","Pat","Krista","Roxanne","Angelica","Johnnie","Robyn","Francis","Adrienne","Rosalie","Alexandra","Brooke","Bethany","Sadie","Bernadette","Traci","Jody","Kendra","Jasmine","Nichole","Rachael","Chelsea","Mable","Ernestine","Muriel","Marcella","Elena","Krystal","Angelina","Nadine","Kari","Estelle","Dianna","Paulette","Lora","Mona","Doreen","Rosemarie","Angel","Desiree","Antonia","Hope","Ginger","Janis","Betsy","Christie","Freda","Mercedes","Meredith","Lynette","Teri","Cristina","Eula","Leigh","Meghan","Sophia","Eloise","Rochelle","Gretchen","Cecelia","Raquel","Henrietta","Alyssa","Jana","Kelley","Gwen","Kerry","Jenna","Tricia","Laverne","Olive","Alexis","Tasha","Silvia","Elvira","Casey","Delia","Sophie","Kate","Patti","Lorena","Kellie","Sonja","Lila","Lana","Darla","May","Mindy","Essie","Mandy","Lorene","Elsa","Josefina","Jeannie","Miranda","Dixie","Lucia","Marta","Faith","Lela","Johanna","Shari","Camille","Tami","Shawna","Elisa","Ebony","Melba","Ora","Nettie","Tabitha","Ollie","Jaime","Winifred","Kristie","Marina","Alisha","Aimee","Rena","Myrna","Marla","Tammie","Latasha","Bonita","Patrice","Ronda","Sherrie","Addie","Francine","Deloris","Stacie","Adriana","Cheri","Shelby","Abigail","Celeste","Jewel","Cara","Adele","Rebekah","Lucinda","Dorthy","Chris","Effie","Trina","Reba","Shawn","Sallie","Aurora","Lenora","Etta","Lottie","Kerri","Trisha","Nikki","Estella","Francisca","Josie","Tracie","Marissa","Karin","Brittney","Janelle","Lourdes","Laurel","Helene","Fern","Elva","Corinne","Kelsey","Ina","Bettie","Elisabeth","Aida","Caitlin","Ingrid","Iva","Eugenia","Christa","Goldie","Cassie","Maude","Jenifer","Therese","Frankie","Dena","Lorna","Janette","Latonya","Candy","Morgan","Consuelo","Tamika","Rosetta","Debora","Cherie","Polly","Dina","Jewell","Fay","Jillian","Dorothea","Nell","Trudy","Esperanza","Patrica","Kimberley","Shanna","Helena","Carolina","Cleo","Stefanie","Rosario","Ola","Janine","Mollie","Lupe","Alisa","Lou","Maribel","Susanne","Bette","Susana","Elise","Cecile","Isabelle","Lesley","Jocelyn","Paige","Joni","Rachelle","Leola","Daphne","Alta","Ester","Petra","Graciela","Imogene","Jolene","Keisha","Lacey","Glenna","Gabriela","Keri","Ursula","Lizzie","Kirsten","Shana","Adeline","Mayra","Jayne","Jaclyn","Gracie","Sondra","Carmela","Marisa","Rosalind","Charity","Tonia","Beatriz","Marisol","Clarice","Jeanine","Sheena","Angeline","Frieda","Lily","Robbie","Shauna","Millie","Claudette","Cathleen","Angelia","Gabrielle","Autumn","Katharine","Summer","Jodie","Staci","Lea","Christi","Jimmie","Justine","Elma","Luella","Margret","Dominique","Socorro","Rene","Martina","Margo","Mavis","Callie","Bobbi","Maritza","Lucile","Leanne","Jeannine","Deana","Aileen","Lorie","Ladonna","Willa","Manuela","Gale","Selma","Dolly","Sybil","Abby","Lara","Dale","Ivy","Dee","Winnie","Marcy","Luisa","Jeri","Magdalena","Ofelia","Meagan","Audra","Matilda","Leila","Cornelia","Bianca","Simone","Bettye","Randi","Virgie","Latisha","Barbra","Georgina","Eliza","Leann","Bridgette","Rhoda","Haley","Adela","Nola","Bernadine","Flossie","Ila","Greta","Ruthie","Nelda","Minerva","Lilly","Terrie","Letha","Hilary","Estela","Valarie","Brianna","Rosalyn","Earline","Catalina","Ava","Mia","Clarissa","Lidia","Corrine","Alexandria","Concepcion","Tia","Sharron","Rae","Dona","Ericka","Jami","Elnora","Chandra","Lenore","Neva","Marylou","Melisa","Tabatha","Serena","Avis","Allie","Sofia","Jeanie","Odessa","Nannie","Harriett","Loraine","Penelope","Milagros","Emilia","Benita","Allyson","Ashlee","Tania","Tommie","Esmeralda","Karina","Eve","Pearlie","Zelma","Malinda","Noreen","Tameka","Saundra","Hillary","Amie","Althea","Rosalinda","Jordan","Lilia","Alana","Gay","Clare","Alejandra","Elinor","Michael","Lorrie","Jerri","Darcy","Earnestine","Carmella","Taylor","Noemi","Marcie","Liza","Annabelle","Louisa","Earlene","Mallory","Carlene","Nita","Selena","Tanisha","Katy","Julianne","John","Lakisha","Edwina","Maricela","Margery","Kenya","Dollie","Roxie","Roslyn","Kathrine","Nanette","Charmaine","Lavonne","Ilene","Kris","Tammi","Suzette","Corine","Kaye","Jerry","Merle","Chrystal","Lina","Deanne","Lilian","Juliana","Aline","Luann","Kasey","Maryanne","Evangeline","Colette","Melva","Lawanda","Yesenia","Nadia","Madge","Kathie","Eddie","Ophelia","Valeria","Nona","Mitzi","Mari","Georgette","Claudine","Fran","Alissa","Roseann","Lakeisha","Susanna","Reva","Deidre","Chasity","Sheree","Carly","James","Elvia","Alyce","Deirdre","Gena","Briana","Araceli","Katelyn","Rosanne","Wendi","Tessa","Berta","Marva","Imelda","Marietta","Marci","Leonor","Arline","Sasha","Madelyn","Janna","Juliette","Deena","Aurelia","Josefa","Augusta","Liliana","Young","Christian","Lessie","Amalia","Savannah","Anastasia","Vilma","Natalia","Rosella","Lynnette","Corina","Alfreda","Leanna","Carey","Amparo","Coleen","Tamra","Aisha","Wilda","Karyn","Cherry","Queen","Maura","Mai","Evangelina","Rosanna","Hallie","Erna","Enid","Mariana","Lacy","Juliet","Jacklyn","Freida","Madeleine","Mara","Hester","Cathryn","Lelia","Casandra","Bridgett","Angelita","Jannie","Dionne","Annmarie","Katina","Beryl","Phoebe","Millicent","Katheryn","Diann","Carissa","Maryellen","Liz","Lauri","Helga","Gilda","Adrian","Rhea","Marquita","Hollie","Tisha","Tamera","Angelique","Francesca","Britney","Kaitlin","Lolita","Florine","Rowena","Reyna","Twila","Fanny","Janell","Ines","Concetta","Bertie","Alba","Brigitte","Alyson","Vonda","Pansy","Elba","Noelle","Letitia","Kitty","Deann","Brandie","Louella","Leta","Felecia","Sharlene","Lesa","Beverley","Robert","Isabella","Herminia","Terra","Celina"};
    if (!getANameDB.count(p))
    {
        string name = "";
        runSRand();

        if (typeOfName == ALGORITHM_GENERATED)
        {
            string consonants = "bcdfghjklmnpqrstvxwz";
            string vocals = "aeiouy";


            while (AlgoGenMaxSyllables-- > 0)
                name += consonants.substr(rand() % consonants.size(), 1) + "" + vocals.substr(rand() % vocals.size(), 1);
        }
        else
        {   
            

            if (rand() % 1 == 1)
                name = male_names_list[rand() % male_names_list.size()];
            else
                name = female_names_list[rand() % female_names_list.size()];

            name += " "+surnames_list[rand() % surnames_list.size()];


        }

        getANameDB[p] = name;
    }

    return getANameDB[p];
}

string Utils::getNestedExceptionText(exception &e, string prefix, int level)
{

    string ret;
    for (int c = 0; c < level; c++)
        ret += " ";
    //ret += "-> ";
    ret += prefix + e.what();

    try{
        rethrow_if_nested(e);
    }
    catch(exception &e2)
    {
        ret += "\n" + getNestedExceptionText(e2, prefix, level+1);
    }

    return ret;
}