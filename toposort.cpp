void solve(int n, vector<int> adj[], stack<int> &st, vector<int> &vis)
	{
	    vis[n]=1 ;
	    for(auto it : adj[n])
	    {
	        if(vis[it] == 0) solve(it,adj,st,vis) ;
	    }
	    
	    st.push(n) ;
	}
	
	vector<int> topoSort(int V, vector<int> adj[]) 
	{
	    // code here
	    vector<int> vis(V,0) , ans(V,0);
	  
	    queue<int> q ;
	   
	   for(int i = 0 ; i < V ; i++)
	   {
	       
	       
	       if(vis[i]==0)
	       {
	        q.push(i) ;
	    
	    while(!q.empty())
	    {
	        int n = q.front() ;
	        q.pop() ;
	        for(auto it : adj[n])
	        {
	            if(vis[it]==0)
	            {
	             ans[it]+=1 ;
	            }  
	        }
	    }
	    
	    
	       }
	    
	   }
	   queue<int> qu ;
	   for(int i = 0 ; i < V ; i++)
	   {
	       if(ans[i]==0)
	       {
	           ans[i]= -1 ;
	           qu.push(i) ;
	       }
	   }
	   vis.clear() ;
	   while(!qu.empty())
	   {
	       int n = qu.front() ;
	       qu.pop() ;
	       for(auto it : adj[n])
	       {
	           ans[it] = ans[it]-1 ;
	           
	       }
	  for(int i = 0 ; i < V ; i++)
	   {
	       if(ans[i]==0)
	       {
	           ans[i] = -1 ;
	           qu.push(i) ;
	       }
	   }
	   
	   vis.push_back(n) ;
	       
	   }
	   
	   
	   return vis;
	}
