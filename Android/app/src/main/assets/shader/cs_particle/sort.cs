#version VERSION

#define eLocalBms      0U
#define eLocalDisperse 1U
#define eBigFlip       2U
#define eBigDisperse   3U

precision mediump float;

layout(local_size_x = LOCAL_SIZE_X) in;

layout(std140, binding=1) buffer Position
{
    vec4 Positions[];
};

layout(std140, binding=2) buffer Velocity
{
    vec4 Velocities[];
};

layout(std140, binding=3) buffer Color
{
    vec4 Colors[];
};

layout(std140, binding=4) buffer Lifetime
{
    vec4 Lifetimes[];
};

uniform uint uAlgorithm;
uniform uint uN;
uniform uint uH;


// Workgroup local memory. We use this to minimise round-trips to global memory.
// It allows us to evaluate a sorting network of up to 1024 with one shader invocation.
shared vec4 local_lifetimes[LOCAL_SIZE_X * 2];
shared vec4 local_positions[LOCAL_SIZE_X * 2];
shared vec4 local_velocity[LOCAL_SIZE_X * 2];
shared vec4 local_colors[LOCAL_SIZE_X * 2];

void SwapLifetimes(uint x, uint y)
{
    vec4 tmp = Lifetimes[x];
    Lifetimes[x] = Lifetimes[y];
    Lifetimes[y] = tmp;
}

void SwapPositions(uint x, uint y)
{
    vec4 tmp = Positions[x];
    Positions[x] = Positions[y];
    Positions[y] = tmp;
}

void SwapColors(uint x, uint y)
{
    vec4 tmp = Colors[x];
    Colors[x] = Colors[y];
    Colors[y] = tmp;
}

void SwapVelocities(uint x, uint y)
{
    vec4 tmp = Velocities[x];
    Velocities[x] = Velocities[y];
    Velocities[y] = tmp;
}

void SwapLocalLifetimes(uint x, uint y)
{
    vec4 tmp = local_lifetimes[x];
    local_lifetimes[x] = local_lifetimes[y];
    local_lifetimes[y] = tmp;
}

void SwapLocalPositions(uint x, uint y)
{
    vec4 tmp = local_positions[x];
    local_positions[x] = local_positions[y];
    local_positions[y] = tmp;
}

void SwapLocalColors(uint x, uint y)
{
    vec4 tmp = local_colors[x];
    local_colors[x] = local_colors[y];
    local_colors[y] = tmp;
}

void SwapLocalVelocities(uint x, uint y)
{
    vec4 tmp = local_velocity[x];
    local_velocity[x] = local_velocity[y];
    local_velocity[y] = tmp;
}

void global_compare_and_swap(uvec2 idx){
	if (Lifetimes[idx.x].x < Lifetimes[idx.y].x)
    {
        SwapLifetimes(idx.x, idx.y);
        SwapPositions(idx.x, idx.y);
        SwapVelocities(idx.x, idx.y);
        SwapColors(idx.x, idx.y);
	}
}

void big_flip( in uint n, in uint h) {

	//	uint n                  // total number of sortable elements
	//	uint h                  // flip height
	//	uint gl_WorkGroupSize.x // number of threads in block/workgroup: 
                                // each thread deals with two sortable elements

	if (gl_WorkGroupSize.x * 2U > h) {
		return;
	}

	uint t_prime = gl_GlobalInvocationID.x;
	uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 

	uint q       = ((2U * t_prime) / h) * h;
	uint x       = q     + (t_prime % half_h);
	uint y       = q + h - (t_prime % half_h) - 1U; 


	global_compare_and_swap(uvec2(x,y));
}


void big_disperse( in uint n, in uint h ) {

	//	uint n                  // total number of sortable elements
	//	uint h                  // disperse height
	//	uint gl_WorkGroupSize.x // number of threads in block/workgroup: 
                                // each thread deals with two sortable elements

	if ( gl_WorkGroupSize.x * 2U > h ) {
		return;
	};

	uint t_prime = gl_GlobalInvocationID.x;

	uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 

	uint q       = ((2U * t_prime) / h) * h;
	uint x       = q + (t_prime % (half_h));
	uint y       = q + (t_prime % (half_h)) + half_h;

	global_compare_and_swap(uvec2(x,y));

}

// Performs compare-and-swap over elements held in shared,
// workgroup-local memory
void local_compare_and_swap(uvec2 idx){
	if (local_lifetimes[idx.x].x < local_lifetimes[idx.y].x)
    {
        SwapLocalLifetimes(idx.x, idx.y);
        SwapLocalPositions(idx.x, idx.y);
        SwapLocalVelocities(idx.x, idx.y);
        SwapLocalColors(idx.x, idx.y);
	}
}

// Performs full-height flip (h height) over locally available indices.
void local_flip(in uint h){
		uint t = gl_LocalInvocationID.x;
		barrier();

		uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 
		uvec2 indices = 
			uvec2( h * ( ( 2U * t ) / h ) ) +
			uvec2( t % half_h, h - 1U - ( t % half_h ) );

		local_compare_and_swap(indices);
}

// Performs progressively diminishing disperse operations (starting with height h)
// on locally available indices: e.g. h==8 -> 8 : 4 : 2.
// One disperse operation for every time we can half h.
void local_disperse(in uint h){
	uint t = gl_LocalInvocationID.x;
	for ( ; h > 1U ; h /= 2U ) {
		
		barrier();

		uint half_h = h >> 1; // Note: h >> 1 is equivalent to h / 2 
		uvec2 indices = 
			uvec2( h * ( ( 2U * t ) / h ) ) +
			uvec2( t % half_h, half_h + ( t % half_h ) );

		local_compare_and_swap(indices);
	}
}

// Perform binary merge sort for local elements, up to a maximum number 
// of elements h.
void local_bms(uint h){
	uint t = gl_LocalInvocationID.x;
	for ( uint hh = 2U; hh <= h; hh <<= 1U ) {  // note:  h <<= 1 is same as h *= 2
		local_flip( hh);
		local_disperse( hh/2U);
	}
}

void main(){

	// this shader can be called in four different modes:
	// 1. local flip+disperse (up to n == local_size_x * 2) 
	// 2. big flip
	// 3. big disperse
	// 4. local disperse 
	// the total number of elements 

	uint t = gl_LocalInvocationID.x;

    // Calculate global offset for local workgroup
    //
	uint offset = gl_WorkGroupSize.x * 2U * gl_WorkGroupID.x; 

	if (uAlgorithm <= eLocalDisperse){
		// pull to local memory
	    // Each local worker must save two elements to local memory, as there
	    // are twice as many elments as workers.
		local_lifetimes[t*2U]   = Lifetimes[offset+t*2U];
		local_lifetimes[t*2U+1U] = Lifetimes[offset+t*2U+1U];
        local_positions[t*2U]   = Positions[offset+t*2U];
		local_positions[t*2U+1U] = Positions[offset+t*2U+1U];
        local_velocity[t*2U]   = Velocities[offset+t*2U];
		local_velocity[t*2U+1U] = Velocities[offset+t*2U+1U];
        local_colors[t*2U]   = Colors[offset+t*2U];
		local_colors[t*2U+1U] = Colors[offset+t*2U+1U];
	}

	uint n = uN;

	// check which one of these does not work properly...
	// it could also be a synchronisation problem!!!

	switch (uAlgorithm){
		case eLocalBms:
			local_bms(uH);
		break;
		case eLocalDisperse:
			local_disperse(uH);
		break;
		case eBigFlip:
			big_flip(uN, uH);
		break;
		case eBigDisperse:
			big_disperse(uN, uH);
		break;
	}

	// Write local memory back to buffer

	if (uAlgorithm <= eLocalDisperse){
		barrier();
		// push to global memory
		Lifetimes[offset+t*2U]   = local_lifetimes[t*2U];
		Lifetimes[offset+t*2U+1U] = local_lifetimes[t*2U+1U];
        Positions[offset+t*2U]   = local_positions[t*2U];
		Positions[offset+t*2U+1U] = local_positions[t*2U+1U];
        Velocities[offset+t*2U]   = local_velocity[t*2U];
		Velocities[offset+t*2U+1U] = local_velocity[t*2U+1U];
        Colors[offset+t*2U]   = local_colors[t*2U];
		Colors[offset+t*2U+1U] = local_colors[t*2U+1U];
	}
}