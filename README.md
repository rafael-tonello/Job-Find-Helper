# Job Find Helper
Job Find Helper is a little program I developed to help me in finding a new job. I decided to write it after a few days in which I had to to open and search for jobs oportunities several times.

This program does that job for me. It searchs for vacancies on some job portals and call some triggers to notify-me.

# How it works
This program periodically downloads some job portals webpages and analyses their html content to extract job oportunities and print them on terminal or call some triggers with the found jobs informations. 

Internaly, for each job portal, the system contains a 'driver' that performs the "find job process" every few minutes. The "job find process" is the process of download the html of a job portal, extract jobs from that html, checking whether the jobs found are new, and thus, send them out to outside world.

# Triggers
Triggers are commands that can be passed to the Job Find Helper, which are executed whena  new job opportunity is found.
You are not required to inform triggers to the app. If you do not speciy any triggers, the app will print the job details to stdout (the app will also try to adjust the loglevel of console output to try keep the stdout cleaner).

# Command line example

Specifying jobs searchs on Linkeding and Net-empregos.com and calling a bash script with the found data.

```bash
    ./jobfindhelper \
    -c "./notifysend.sh \"#title#\" \"#company#\" \"#location#\" \"#url#\" \"#logo#\"" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=programador&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=desenvolvedor&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=c%2B%2B&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=.net&cidade=&categoria=0&zona=0&tipo=0"
    -u  "\"https://www.linkedin.com/jobs/search/?currentJobId=3651841009&geoId=100364837&keywords=c%2B%2B&location=Portugal&refresh=true\"" \
    -u "\"https://www.linkedin.com/jobs/search/?currentJobId=3612744837&geoId=100364837&keywords=node.js&location=Portugal&refresh=true\"" \
    -u "\"https://www.linkedin.com/jobs/search/?currentJobId=3651841009&geoId=100364837&keywords=C%23%20&location=Portugal&refresh=true\"" \
    -u "\"https://www.linkedin.com/jobs/search/?currentJobId=3645253255&geoId=100364837&keywords=golang&location=Portugal&refresh=true\""

```
    Lets take a look at some parts of the command above:
        ./jobfindhelper
            is the name of the executable.

        -c "./notifysend.sh ...
            the '-c' argument is used do specify a command to be runned when a jobs is found. The placeholders (words between ##) will be replace by the jobs information when the command is ready to be execute. The available place holds are described in the '--help ouput' section.

        -u "https://www.net-empregos.com/pesquisa-empregos ...

            the '-u' argument is used to specify custom search urls. Using the domain name, the Job Finde Helper will try to find the correct driver to process the HTMLs of respective domain.

# --help output
Bellow you can se the '--help' of the Job Finder Helper

    Net-Empregos monitor and trigger, version 0.2.0
    Net-Empregos monitor. Job oportunities monitoring and command trigger.

    Usage: jobfindhelper [options]

    Options: 
        -h, --help       Displays this help text

        -u, --url        A net-empregos valid url with a job list.
                        you can use a search url, or a category url.
                        you can use this argument multiple times.
                        If you do not specify any url, the system will work.
                        with predefined ones. The predefined urls will look.
                        look for only for T.I. jobs.

        -c, --command    A cmd to be execute when a new is found. Some
                        placeholders can be used on commmand execution:
                            #title# -> is replaced by the job title
                            #url# -> is replaced by the job url
                            #company# -> is replaced by the company name
                            #logo# -> is replaced by the url of the company logo
                            #place# -> is replaced by the url of the place of job
                            #category# -> is replaced by the url of the job
                                        category

        --log-level      Specify the output log level of the app. You can use:
                                debug: all messages will be displayed
                            info: Info, warnings and erros will be showed
                            warning: Only warnings and erros will be displayed.
                                    Note that the normal app output (info) will
                                    be also supressed
                            error: Only error messages will be displayed.
                        This option only changes the terminal output (stdout). The
                        log file ever will receive all outputs texts from the app.


    Options for portal monitor services: 
    --NetEmpregos_enabled   Enable or disable the support for Net-empregos.com
                            portal. Options:
                                yes: enable the NetEmpregos pulling service
                                no: disable the NetEmpregos pulling service

    --ItJobs_enabled        Enable or disable the support for ItJobs.pt portal.
                            Options:
                                yes: enable the ItJobs pulling service
                                no: disable the ItJobs pulling service

    --linkedin_enabled      Enable or disable the support for linkedin.
                            Options:
                                yes: enable the Linkedin pulling service
                                no: disable the Linkedin pulling service

    --Indeed_enabled        Enable or disable the support for Indeed.com
                            portal. Options:
                                yes: enable the Indeed pulling service
                                no: disable the Indeed pulling service




    # Task lists
    ## general
        [ ] If urls are provided, run only services for these urls
        [ ] Add DependecyInjectionManager class to the project
        [ ] Load job details methods (job finder services)

    ## Portals do add
    ### Portugal:
        [x] Net-empregos
        [x] itjobs.pt
        [ ] https://www.computerworld.com.pt/
        [ ] https://netemprego.com/index.php?auth_sess=1eu62vbj312upmn3deoiukmomn&ref=c84e2a02577b50bed39467af9

    ### Europe:
        [ ] cvwarehouse.com/

    ### Brazil:
        [ ] https://vagas.byintera.com/
        
