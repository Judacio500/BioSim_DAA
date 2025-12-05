int criteria(PERSON *p1, PERSON *p2, int eval)
{
    switch(eval)
    {
        case 1:
            return (p1->infRisk > p2->infRisk);

        case 2:
            return (p1->daysInfected > p2->daysInfected);

        case 3: 
            return (strcmp(p1->name, p2->name) < 0);
            
        case 4:
            return (p1->id < p2->id);
            
        default: 
            return 0;
    }
}