library(shiny)
library(ggplot2)

sessSimData <- NULL

# Define UI for application that draws a histogram
ui <- fluidPage(
    headerPanel(
        title="ML Stim",
        windowTitle = "Hola"
    ),
    sidebarLayout(
        sidebarPanel(
            width=2,
            selectInput('selectSim','Select Sim Profile',choice = list.files('Sims/')),
            textOutput("name"),
            textOutput("dateCreated"),
            textOutput("dateModified"),
            textOutput("dateLastRun"),
            br(),
            radioButtons(
                inputId="yPlots",
                label="Graph Data:",
                choices=c("Voltage"="v",
                          "n"="n",
                          "m"="m",
                          "h"="h",
                          "vExt"="vExt")
            ),
            br(),
            numericInput(inputId="vNum", label="Voltage Container", value=1),
            numericInput(inputId="nNum", label="u Container", value=1),
            numericInput(inputId="mNum", label="n Container", value=1),
            numericInput(inputId="hNum", label="h Container", value=1),
            numericInput(inputId="vExtNum", label="vExt Container", value=1),
            br(),
            actionButton(
                inputId="loadBtn",
                label="Load"
            )
        ),
        mainPanel(
            plotOutput(
                outputId="graph"
            )
        )
    )
)

loadPlot <- function(input, output, session){
    output$graph <- renderPlot(
        {
            mul <- 0
            container <- 1
            if(isolate(input$yPlots) == "v"){
                container <- isolate(input$vNum)
            }
            else if(isolate(input$yPlots) == "n"){
                mul <- 1
                container <- isolate(input$nNum)
            }
            else if(isolate(input$yPlots) == "m"){
                mul <- 2
                container <- isolate(input$mNum)
            }
            else if (isolate(input$yPlots) == "h"){
                mul <- 3
                container <- isolate(input$hNum)
            }
            else if (isolate(input$yPlots) == "vExt"){
                mul <- 4
                container <- isolate(input$vExtNum)
            }
            
            simData <<- read.csv(file=paste0("Sims/", isolate(input$selectSim), "/output.csv"), header=FALSE, sep=",")
            # SimData <- sessSimData
            p <- ggplot() + geom_path(aes(x=simData[,1],y=simData[,(51 * mul) + container + 1]))
            return(p)
        }
    )
}

updatePlot <- function(input, output, session){
    output$graph <- renderPlot(
        {
            mul <- 0
            container <- 1
            if(isolate(input$yPlots) == "v"){
                container <- isolate(input$vNum)
            }
            else if(isolate(input$yPlots) == "n"){
                mul <- 1
                container <- isolate(input$nNum)
            }
            else if(isolate(input$yPlots) == "m"){
                mul <- 2
                container <- isolate(input$mNum)
            }
            else if (isolate(input$yPlots) == "h"){
                mul <- 3
                container <- isolate(input$hNum)
            }
            else if (isolate(input$yPlots) == "vExt"){
                mul <- 4
                container <- isolate(input$vExtNum)
            }
            
            # simData <- session$sessSimData
            if (exists("simData")){
                p <- ggplot() + geom_path(aes(x=simData[,1],y=simData[,(51 * mul) + container + 1]))
                return(p)   
            }
            return("Nope")
        }
    )
}

# 
server <- function(input, output, session) {
    
    observeEvent(input$loadBtn,{
        loadPlot(input, output, session)
    })
    
    observeEvent(input$yPlots,{
        updatePlot(input, output, session)
    })
    observeEvent(input$vNum,{
        updatePlot(input, output, session)
    })
    observeEvent(input$nNum,{
        updatePlot(input, output, session)
    })
    observeEvent(input$mNum,{
        updatePlot(input, output, session)
    })
    observeEvent(input$hNum,{
        updatePlot(input, output, session)
    })
    
    output$simSelected <- renderText({
        paste0('You have selected: ', input$selectSim)
    })
    
    output$name <- renderText(
        {
            info = read.csv(file=paste0("Sims/", input$selectSim, "/info.csv"), header=FALSE, sep=",")
            return (paste0("Simulation Name: ", toString(info[info[1]=="name", 2])))
        }
        
    )
    output$dateCreated <- renderText(
        {
            info = read.csv(file=paste0("Sims/", input$selectSim, "/info.csv"), header=FALSE, sep=",")
            return (paste0("Date Created: ", toString(info[info[1]=="dateCreated", 2])))
        }
    )
    output$dateLastRun <- renderText(
        {
            info = read.csv(file=paste0("Sims/", input$selectSim, "/info.csv"), header=FALSE, sep=",")
            return (paste0("Date Last Run: ", toString(info[info[1]=="dateLastRun", 2])))
        }
    )
    output$dateModified <- renderText(
        {
            info = read.csv(file=paste0("Sims/", input$selectSim, "/info.csv"), header=FALSE, sep=",")
            return (paste0("Date Modified: ", toString(info[info[1]=="dateModified", 2])))
        }
    )
    # simData = read.csv(file=paste0("Sims/", input$selectSim, "/output.csv"), header=FALSE, sep=",")
    
}

# Run the application 
shinyApp(ui = ui, server = server)